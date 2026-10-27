/* Basara 4 Android - CPU Interface Implementation */
/*
 * This file provides the CPU interface between the guest (x86_64 game code)
 * and the host (ARM64 Android). On Android, we use FEXCore for JIT translation.
 */
#define _GNU_SOURCE
#include "runtime.h"
#include "guest_cpu.h"

#if defined(GUEST_CPU_FEX)
/* FEXCore backend - implemented in cpu/guest_cpu_fex.cpp */
extern void guest_cpu_init(void *(*map)(size_t size, int prot));
extern void guest_cpu_code(uintptr_t base, size_t size);
extern uintptr_t guest_cpu_host_function(void *fn);
extern uint64_t guest_cpu_call(uintptr_t fn, unsigned count, const uint64_t *args);
extern void guest_cpu_thread_stack(void *top, size_t size);
extern void guest_cpu_set_gs(void *base);
extern void guest_cpu_abandon(void);
extern void guest_cpu_thread_end(void);
extern uintptr_t guest_cpu_return_address(uintptr_t native);
extern uintptr_t guest_cpu_stack_pointer(void);
extern int guest_cpu_signal_regs(const void *ucontext, GuestRegs *regs);
extern uintptr_t guest_cpu_host_pc(const void *ucontext);
extern int guest_cpu_handle_fault(int sig, siginfo_t *info, void *ucontext);
extern uint64_t guest_cpu_tsc(void);
extern uint64_t guest_cpu_tsc_frequency(void);
extern int guest_cpu_hook(uintptr_t address, size_t size, GuestHook hook);
#else
/* Fallback: no CPU emulation available */
typedef int runtime_cpu_unused;
#endif

/* Expose the guest CPU interface */
/* Functions are implemented in cpu/guest_cpu_fex.cpp for Android */
