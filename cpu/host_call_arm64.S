/* ARM64 assembly for calling host functions from FEXCore JIT */
.section .text
.align 4
.globl basara4cpu_host_call
.type basara4cpu_host_call, %function

/* void basara4cpu_host_call(void* fn, const HostArgs* args, HostResult* result)
 * 
 * x0 = fn (host function pointer)
 * x1 = args (HostArgs pointer)
 * x2 = result (HostResult pointer)
 * 
 * HostArgs layout:
 *   x[0..7]: integer arguments
 *   stack_count: number of stack args
 *   stack: pointer to stack args
 *   v[0..7]: vector arguments
 * 
 * Returns:
 *   result->x[0] = RAX
 *   result->x[1] = RDX
 *   result->v[0] = XMM0
 *   result->v[1] = XMM1
 */
basara4cpu_host_call:
    // Save return address
    stp x29, x30, [sp, -16]!
    
    // Load function pointer
    mov x8, x0
    
    // Load args pointer
    mov x9, x1
    
    // Load result pointer
    mov x10, x2
    
    // Copy integer arguments (x0-x7 from args)
    ldp x0, x1, [x9], #16
    ldp x2, x3, [x9], #16
    ldp x4, x5, [x9], #16
    ldp x6, x7, [x9], #16
    
    // Load stack info
    ldr x11, [x9], #8      // stack_count
    ldr x12, [x9]          // stack pointer
    
    // Copy stack arguments
    // Args 8+ go on stack in AAPCS64 order
    1:
    cmp x11, #0
    beq 2f
    ldp x1, x2, [x12], #16
    sub x11, x11, #2
    b 1b
    2:
    
    // Call host function
    blr x8
    
    // Store results
    // RAX -> result->x[0]
    // RDX -> result->x[1]
    str x0, [x10]
    str x2, [x10, #8]
    
    // Copy vector results (XMM0, XMM1)
    // For simplicity, using MOVQ; full impl would use FPR instructions
    umov w3, v0.d[0]
    umov w4, v0.d[1]
    umov w5, v1.d[0]
    umov w6, v1.d[1]
    
    str w3, [x10, #16]
    str w4, [x10, #20]
    str w5, [x10, #24]
    str w6, [x10, #28]
    
    // Restore and return
    ldp x29, x30, [sp], #16
    ret

.size basara4cpu_host_call, . - basara4cpu_host_call
