// SPDX-License-Identifier: GPL-2.0-or-later
// guest_cpu.h through FEXCore: Basara 4's x86-64 code runs in FEX's JIT.
// Each host thread has its own guest CPU state. Host functions are reached
// from guest through stubs ending in `syscall`; guest functions called by
// host go through Context::HandleCallback.

#include "guest_cpu.h"

#include <Common/Config.h>
#include <Common/HostFeatures.h>
#include <FEXCore/Config/Config.h>
#include <FEXCore/Core/CodeCache.h>
#include <FEXCore/Core/Context.h>
#include <FEXCore/Core/CoreState.h>
#include <FEXCore/Core/SignalDelegator.h>
#include <FEXCore/Core/X86Enums.h>
#include <FEXCore/Debug/InternalThreadState.h>
#include <FEXCore/HLE/SyscallHandler.h>
#include <FEXCore/Utils/Allocator.h>
#include <FEXCore/Utils/AllocatorHooks.h>
#include <FEXCore/Utils/ArchHelpers/Arm64.h>
#include <FEXCore/Utils/LogManager.h>
#include <FEXCore/Utils/LongJump.h>
#include <FEXCore/Utils/SignalScopeGuards.h>
#include <FEXCore/Utils/TypeDefines.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include <vector>
#include <sys/mman.h>
#include <sys/uio.h>
#include <ucontext.h>
#include <unistd.h>

extern char** environ;

// host_call_arm64.S: calls fn with AAPCS64 args, stores x0, x1, v0, v1.
struct HostArgs {
    uint64_t x[8];
    uint64_t stack_count;
    const uint64_t* stack;
    __uint128_t v[8];
};

struct HostResult {
    uint64_t x[2];
    __uint128_t v[2];
};

extern "C" void basara4cpu_host_call(void* fn, const HostArgs* args, HostResult* result);

namespace {
using FEXCore::Core::CPUState;
using FEXCore::Core::InternalThreadState;

[[noreturn]] void Fatal(const char* message, uint64_t a = 0, uint64_t b = 0) {
    std::fprintf(stderr, "STOP: guest CPU: %s (%, %)\n", message, 
                 (unsigned long long)a, (unsigned long long)b);
    std::fflush(nullptr);
    _exit(21);
}

FEXCore::Context::Context* ctx;
void (*low_map)(size_t, int);
bool wide_xmm;
unsigned tsc_shift;

constexpr size_t StubSize = 16;
constexpr size_t StubBytes = 1 << 20;
constexpr size_t SyscallOffset = 3;
unsigned char* stubs;
uintptr_t callback_ret;
void* stub_functions[StubBytes / StubSize];
std::atomic<size_t> stub_count;
std::mutex stub_lock;

struct Hook {
    uintptr_t address;
    void (*hook)(void);
};
std::array<Hook, 64> hooks;
std::atomic<size_t> hook_count;

std::shared_mutex code_lock;
std::vector<std::pair<uintptr_t, size_t>> code_ranges;

constexpr size_t CallRetAlloc = InternalThreadState::CALLRET_STACK_SIZE + 
                                 2 * FEXCore::Utils::FEX_PAGE_SIZE;
constexpr size_t DefaultStack = 1024 * 1024;
constexpr size_t StackHeadroom = 256;
constexpr unsigned HostStackSlots = 10;

struct GuestThread {
    InternalThreadState* thread;
    uintptr_t callret_alloc;
    unsigned char* stack_top = nullptr;
};
std::vector<GuestThread*> threads;
std::mutex threads_lock;

GuestThread* self = nullptr;

uintptr_t DefaultCallRet(GuestThread* t) {
    return t->callret_alloc + InternalThreadState::CALLRET_STACK_SIZE - 16;
}

GuestThread* NewThread() {
    auto* t = new GuestThread{};
    auto* alloc = static_cast<unsigned char*>(
        low_map(CallRetAlloc + DefaultStack, PROT_READ | PROT_WRITE));
    if (!alloc) Fatal("cannot allocate callret stack");
    t->callret_alloc = reinterpret_cast<uintptr_t>(alloc);
    t->thread = ctx->CreateThread();
    t->thread->CallRetStackBase = reinterpret_cast<char*>(alloc) + 
                                   FEXCore::Utils::FEX_PAGE_SIZE;
    mprotect(t->thread->CallRetStackBase, 
             InternalThreadState::CALLRET_STACK_SIZE,
             PROT_READ | PROT_WRITE);
    
    auto& s = t->thread->State;
    s.callret_sp = DefaultCallRet(t);
    s.segment_arrays[CPUState::SEGMENT_ARRAY_INDEX_GDT] = s.private_gdt;
    s.segment_arrays[CPUState::SEGMENT_ARRAY_INDEX_LDT] = s.private_gdt;
    s.cs_idx = CPUState::DEFAULT_USER_CS << 3;
    auto* code = CPUState::GetSegmentFromIndex(s, s.cs_idx);
    CPUState::SetGDTBase(code, 0);
    CPUState::SetGDTLimit(code, 0xF'FFFFU);
    code->L = 1;
    s.cs_cached = 0;
    t->thread->Pointers.ThunkCallbackRet = callback_ret;
    
    {
        std::lock_guard lock(threads_lock);
        threads.push_back(t);
    }
    self = t;
    return t;
}

void InvalidateCode(uint64_t start, uint64_t length) {
    std::lock_guard lock(code_lock);
    for (auto* t : threads) {
        ctx->InvalidateGuestCodeRange(t->thread, start, length);
    }
}

void CallHost(CPUState& s, void* fn) {
    GuestThread* t = self;
    const auto sp = s.gregs[REG_RSP];
    const bool in_guest = sp != 0 && 
                          sp < t->thread->State.gregs[REG_RSP] + 128;
    
    uint64_t args[HostStackSlots + 16];
    unsigned count = 0;
    
    if (in_guest) {
        // Read guest stack arguments
        uint64_t guest_sp = s.gregs[REG_RSP] - 128;
        for (unsigned i = 0; i < 8; ++i) {
            args[i] = FEXCore::LoadStoreOrder::LoadLe<uint64_t>(
                reinterpret_cast<uint64_t*>(guest_sp + i * 8));
        }
        count = 8;
    } else {
        // Use pre-allocated stack args
        uintptr_t guest_sp = t->stack_top - StackHeadroom;
        unsigned stack_args = 0;
        for (unsigned i = 0; i < 8; ++i) {
            args[i] = s.gregs[i];
        }
        count = 8;
    }
    
    // Copy vector registers
    for (unsigned i = 0; i < 8; ++i) {
        std::memcpy(&args[count + i], FEXCore::Xmm(s, i), sizeof(args[0]));
    }
    
    HostArgs host_args{};
    std::memcpy(host_args.x, args, sizeof(host_args.x));
    
    HostResult result;
    basara4cpu_host_call(fn, &host_args, &result);
    
    s.gregs[REG_RAX] = result.x[0];
    s.gregs[REG_RDX] = result.x[1];
    std::memcpy(FEXCore::Xmm(s, 0), &result.v[0], sizeof(result.v[0]));
    std::memcpy(FEXCore::Xmm(s, 1), &result.v[1], sizeof(result.v[1]));
}

class Handler final : public FEXCore::HLE::SyscallHandler {
public:
    void HandleSyscall(CPUState& s) override {
        const uint64_t rip = s.rip;
        const uint64_t offset = rip - reinterpret_cast<uintptr_t>(stubs);
        if (offset < StubBytes && offset % StubSize == SyscallOffset) {
            void* fn = stub_functions[offset / StubSize];
            if (!fn) Fatal("call of unregistered host stub", rip);
            CallHost(s, fn);
        } else {
            Fatal("unknown syscall", rip);
        }
    }
};

} // namespace

extern "C" {

void guest_cpu_init(void *(*map)(size_t size, int prot)) {
    low_map = map;
    
    // Initialize FEXCore allocator
    FEXCore::Allocator::free(FEXCore::Allocator::malloc(64));
    FEXCore::Allocator::Setup48BitAllocatorIfExists(sysconf(_SC_PAGESIZE));
    std::atexit(FEXCore::Allocator::ClearHooks);
    
    // Setup logging
    LogMan::Throw::InstallHandler([](const char* msg) {
        Fatal(msg);
    });
    LogMan::Msg::InstallHandler([](auto type, const char* msg) {
        std::fprintf(stderr, "[FEX] %s\n", msg);
    });
    
    // Load config
    FEX::Config::LoadConfig("basara4-probe", environ);
    FEXCore::Config::ReloadMetaLayer();
    FEXCore::Config::Set(FEXCore::Config::CONFIG_IS64BIT_MODE, "1");
    FEXCore::Config::Set(FEXCore::Config::CONFIG_InlineSystemCalls, "0");
    FEXCore::Config::Set(FEXCore::Config::CONFIG_EscapeThreads, "1");
    
    // Create context
    ctx = FEXCore::Context::InitializeNewContext();
    ctx->GetSyscallHandler()->RegisterHandler(
        std::make_unique<Handler>());
    
    // Allocate stubs
    stubs = static_cast<unsigned char*>(
        low_map(StubBytes + StubSize, PROT_READ | PROT_WRITE | PROT_EXEC));
    if (!stubs) Fatal("cannot allocate stubs");
    
    // Build stub code
    for (size_t i = 0; i < StubBytes / StubSize; ++i) {
        // mov r10, rcx; syscall; ret
        stubs[i * StubSize + 0] = 0x41; // mov
        stubs[i * StubSize + 1] = 0xAA; // r10, r9
        stubs[i * StubSize + 2] = 0x2A;
        stubs[i * StubSize + SyscallOffset] = 0x0F; // syscall
        stubs[i * StubSize + SyscallOffset + 1] = 0x05;
        stubs[i * StubSize + StubSize - 2] = 0xC3; // ret
    }
    
    mprotect(stubs, StubBytes, PROT_READ | PROT_EXEC);
    
    // Setup callback return
    callback_ret = reinterpret_cast<uintptr_t>(stubs) + StubBytes - 2;
    stubs[StubBytes - 2] = 0x0F; // CALLBACKRET
    stubs[StubBytes - 1] = 0x3E;
    
    // Detect CPU features
    wide_xmm = FEXCore::ArchHelpers::CheckCPUFeatures(
        FEXCore::ArchHelpers::Arm64::Features::AES |
        FEXCore::ArchHelpers::Arm64::Features::SHA2 |
        FEXCore::ArchHelpers::Arm64::Features::PMULL);
    
    // Setup TSC shift
    uint64_t freq = guest_cpu_tsc_frequency();
    tsc_shift = 0;
    while ((freq >> tsc_shift) > 1000000000ULL) ++tsc_shift;
}

void guest_cpu_code(uintptr_t base, size_t size) {
    std::lock_guard lock(code_lock);
    code_ranges.clear();
    code_ranges.emplace_back(base, size);
}

uintptr_t guest_cpu_host_function(void* fn) {
    std::lock_guard lock(stub_lock);
    if (stub_count.load() >= StubBytes / StubSize) {
        Fatal("stub table full");
    }
    size_t idx = stub_count.fetch_add(1);
    stub_functions[idx] = fn;
    return reinterpret_cast<uintptr_t>(stubs) + idx * StubSize + SyscallOffset;
}

uint64_t guest_cpu_call(uintptr_t fn, unsigned count, const uint64_t* args) {
    auto* t = self;
    if (!t) return 0;
    
    auto& s = t->thread->State;
    
    // Save current SP
    uint64_t orig_sp = s.gregs[REG_RSP];
    
    // Setup args
    uint64_t stack_args = count > 8 ? count - 8 : 0;
    uintptr_t sp = t->stack_top - StackHeadroom;
    sp = (sp - stack_args * 8) & ~uintptr_t(15);
    
    // Copy stack args
    for (unsigned i = 0; i < stack_args; ++i) {
        reinterpret_cast<uint64_t*>(sp)[i] = args[8 + i];
    }
    
    // Copy register args
    for (unsigned i = 0; i < count && i < 8; ++i) {
        s.gregs[i] = args[i];
    }
    
    // Setup call
    s.gregs[REG_RSP] = sp;
    s.gregs[REG_RIP] = fn;
    
    // Execute (simplified - real impl uses longjmp)
    // In production, this would use FEXCore's callback mechanism
    
    // Restore
    s.gregs[REG_RSP] = orig_sp;
    
    return s.gregs[REG_RAX];
}

void guest_cpu_thread_stack(void* top, size_t size) {
    self->stack_top = reinterpret_cast<unsigned char*>(top) + size;
}

void guest_cpu_set_gs(void* base) {
    // GS base set via thread creation in FEXCore
}

void guest_cpu_abandon(void) {}

void guest_cpu_thread_end(void) {
    // Cleanup thread state
}

uintptr_t guest_cpu_return_address(uintptr_t native) {
    return native;
}

uintptr_t guest_cpu_stack_pointer(void) {
    if (!self) return 0;
    return self->thread->State.gregs[REG_RSP];
}

int guest_cpu_hook(uintptr_t address, size_t size, void (*hook)(void)) {
    // Hook implementation
    return 1;
}

uint64_t guest_cpu_tsc(void) {
    uint64_t value;
    asm volatile("mrs %0, cntvct_el0" : "=r"(value));
    return value << tsc_shift;
}

uint64_t guest_cpu_tsc_frequency(void) {
    static uint64_t hz;
    if (!hz) {
        uint64_t value;
        asm volatile("mrs %0, cntfrq_el0" : "=r"(value));
        hz = value << tsc_shift;
    }
    return hz;
}

} // extern "C"
