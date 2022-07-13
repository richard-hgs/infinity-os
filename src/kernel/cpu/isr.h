#pragma once
#ifndef _ISR_H_
#define _ISR_H_

#include <stdint.h>

#define IRQ0 32		// Temporizador de intervalos 8253/8254 (temporizador do sistema)
#define IRQ1 33		// Teclado
#define IRQ2 34		// Reservada para a 8259B (amarrada ao IRQ 9)
#define IRQ3 35		// COM2 e COM4
#define IRQ4 36		// COM1 e COM3
#define IRQ5 37		// LPT2 ou placa de som
#define IRQ6 38		// Disquetes
#define IRQ7 39		// LPT1
					// ---- PIC secundário ----
#define IRQ8 40		// Relógio de tempo real (real time clock, RTC)
#define IRQ9 41		// Amarrada ao IRQ2
#define IRQ10 42	// Network interface
#define IRQ11 43    // USB port, sound card
#define IRQ12 44	// Mouse PS/2
#define IRQ13 45 	// Co-processador matemático
#define IRQ14 46	// Drives IDE primários
#define IRQ15 47	// Drives IDE secundários

/**
 * @brief ISR - Interrupt Service Routine
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

typedef struct
{
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int eip, cs, eflags;
} IntRegisters;

/**
 * @brief Create an isr_t function type that receives a registers_t* as an argument
 * 
 */
typedef void (*isr_t)(registers_t*);

namespace isr {
	/**
	 * @brief Install and configure the ISR
	 * 
	 */
    void install();

	/**
	 * @brief Register an interrupt handler to given ISR Index. 
	 *        If a handler is assigned to given index it will be replaced.
	 * 
	 * @param isrIndex 	ISR Index
	 * @param handler 	callback handler
	 */
	void registerIsrHandler(uint8_t isrIndex, isr_t handler);
}

#endif