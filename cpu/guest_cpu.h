/* Basara 4 Android - CPU Interface for FEXCore JIT */
#ifndef BASARA4_GUEST_CPU_H
#define BASARA4_GUEST_CPU_H

#include <stddef.h>
#include <stdint.h>
#include <signal.h>

#if defined(__aarch64__)
#define GUEST_CPU_FEX 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC visibility push(default)

/* Guest registers in x86 encoding order. */
enum {
    GUEST_RAX, GUEST_RCX, GUEST_RDX, GUEST_RBX, GUEST_RSP, GUEST_RBP, GUEST_RSI, GUEST_RDI,
    GUEST_R8, GUEST_R9, GUEST_R10, GUEST_R11, GUEST_R12, GUEST_R13, GUEST_R14, GUEST_R15,
    GUEST_GPRS
};

typedef struct {
    uint64_t gpr[GUEST_GPRS];
    uint64_t rip;
} GuestRegs;

/* Signal return codes */
#define GUEST_CPU_IN_GUEST   1
#define GUEST_CPU_IN_HOST    0
#define GUEST_CPU_DEAD       -1

/* Once, before any other call. `map` allocates memory the guest may hold pointers to. */
void guest_cpu_init(void *(*map)(size_t size, int prot));

/* Guest code lives in [base, base + size) (the image, import traps). */
void guest_cpu_code(uintptr_t base, size_t size);

/* The guest-callable address of host function `fn`.
 * SysV ABI on guest side, max 14 integer + 8 vector args. */
uintptr_t guest_cpu_host_function(void *fn);

/* Calls guest function `fn` with `count` integer arguments; returns RAX. */
uint64_t guest_cpu_call(uintptr_t fn, unsigned count, const uint64_t *args);

/* Set the guest stack for this host thread: [top - size, top). */
void guest_cpu_thread_stack(void *top, size_t size);

/* Set guest GS base (TCB) for this host thread. */
void guest_cpu_set_gs(void *base);

/* Thread left guest code via longjmp (pthread_exit). */
void guest_cpu_abandon(void);

/* This host thread is ending: release guest CPU state. */
void guest_cpu_thread_end(void);

/* Return address of the guest call that entered current host function. */
uintptr_t guest_cpu_return_address(uintptr_t native);

/* Guest stack pointer of innermost guest call into host, or 0. */
uintptr_t guest_cpu_stack_pointer(void);

/* Read guest registers at a signal. Returns GUEST_CPU_* constant. */
int guest_cpu_signal_regs(const void *ucontext, GuestRegs *regs);

/* Get host PC at signal. */
uintptr_t guest_cpu_host_pc(const void *ucontext);

/* Handle a fault. Return 1 if handled, 0 to pass through. */
int guest_cpu_handle_fault(int sig, siginfo_t *info, void *ucontext);

/* Get current TSC value (guest view). */
uint64_t guest_cpu_tsc(void);

/* Get TSC frequency. */
uint64_t guest_cpu_tsc_frequency(void);

/* Hook a guest memory region to redirect to host function.
 * Returns 1 on success. */
int guest_cpu_hook(uintptr_t address, size_t size, void (*hook)(void));

#ifdef __cplusplus
}
#endif

#endif /* BASARA4_GUEST_CPU_H */
