#pragma once
#ifndef _ISR_H_
#define _ISR_H_

#include <stdint.h>
/**
 * @brief ISR Behind the scenes
 * 
 * If an interrupt occurred in userspace (actually in a different privilege level), CPU does the following:
 *  - Temporarily saves (internally) the current contents of the SS, ESP, EFLAGS, CS and EIP registers.
 *  - Loads the segment selector and the stack pointer for the new stack (that is, the stack for the privilege level being called) from the TSS into the SS and ESP registers and switches to the new stack.
 *  - Pushes the temporarily saved SS, ESP, EFLAGS, CS, and EIP values for the interrupted procedure’s stack onto the new stack.
 *  - Pushes an error code on the new stack (if appropriate).
 *  - Loads the segment selector for the new code segment and the new instruction pointer (from the interrupt gate or trap gate) into the CS and EIP registers, respectively.
 *  - If the call is through an interrupt gate, clears the IF flag in the EFLAGS register.
 *  - Begins execution of the handler procedure at the new privilege level.
 * 
 * If an interrupt occurred in kernel space (actually in same privilege level), CPU does the following: 
 *  - Push the current contents of the EFLAGS, CS, and EIP registers (in that order) on the stack.
 *  - Push an error code (if appropriate) on the stack.
 *  - Load the segment selector for the new code segment and the new instruction pointer (from the interrupt gate or trap gate) into the CS and EIP registers, respectively.
 *  - Clear the IF flag in the EFLAGS, if the call is through an interrupt gate.
 *  - Begin execution of the handler procedure.
 */

/* Struct which aggregates many registers. */
typedef struct registers {
	uint32_t ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed)) registers_t;

namespace isr {
    void install();
}

#endif