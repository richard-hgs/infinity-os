// libc
#include <stdint.h>
// cpu
#include "idt.h"
#include "pic.h"
#include "isr.h"
// stdlibs
#include "stdio.h"      // Debug only
#include "stdlib.h"
// sys
#include "syscalls.h"
// process
#include "scheduler.h"

extern "C" unsigned int kernelESP; // Imported from scheduler.cpp

#define IDT_MESSAGES_LEN 32
#define INTERRUPT_HANDLERS_SIZE 256

isr_t interruptHandlers[INTERRUPT_HANDLERS_SIZE];

const char* idtMessages[IDT_MESSAGES_LEN] {                        // 627 length
    "Fault - (DE) Divide Error",                                    // 25 length
    "Fault/Trap - (DB) Intel Debug",                                // 29 length
    "Interrupt - (NMI) Interrupt",                                  // 27 length
    "Trap - (BP) Break Point",                                      // 23 length
    "Trap - (OF) Overflow",                                         // 20 length
    "Fault - (BR) Bound Range Exceeded",                            // 33 length
    "Fault - (UD) Invalid Opcode (Undefined Opcode)",               // 46 length
    "Fault - (NM) Device not available (No Math Coprocessor)",      // 55 length
    "Abort - (DF) Double Fault",                                    // 25 length
    "Fault - (CSO) Coprocessor Segment Overrun (reserved)",         // 52 length
    "Fault - (TS) Invalid TSS",                                     // 24 length
    "Fault - (NP) Segment Not Present",                             // 32 length
    "Fault - (SS) Stack-Segment Fault",                             // 32 length
    "Fault - (GP) General Protection",                              // 31 length
    "Fault - (PF) Page Fault",                                      // 23 length
    0, // 15 - (IR) Intel Reserved                                  // 0 length  
    "Fault - (MF) x87 FPU Floating-Point Error (Math Fault)",       // 54 length
    "Fault - (AC) Alignment Check",                                 // 28 length
    "Abort - (MC) Machine Check",                                   // 26 length
    "Fault - (XM) SIMD Floating-Point Exception",                   // 42 length
    0, // 20 - (IR) Intel Reserved
    0, // 21 - (IR) Intel Reserved
    0, // 22 - (IR) Intel Reserved
    0, // 23 - (IR) Intel Reserved
    0, // 24 - (IR) Intel Reserved
    0, // 25 - (IR) Intel Reserved
    0, // 26 - (IR) Intel Reserved
    0, // 27 - (IR) Intel Reserved
    0, // 28 - (IR) Intel Reserved
    0, // 29 - (IR) Intel Reserved
    0, // 30 - (IR) Intel Reserved
    0  // 31 - (IR) Intel Reserved
};

extern "C" void* isr_stub_table[];  // Reference to ISR function pointers

extern "C" void isr_handler(registers_t* r) { // INTEL - ISR Handler
    // Print the interruption cause
    stdio::kprintf("ISR(%d) - ERR_CODE(%d) - %s\n", r->int_no, r->err_code, IFNULL(r->int_no < IDT_MESSAGES_LEN ? idtMessages[r->int_no] : "User - (UI) User interruption", "Reserved - (IR) Intel Reserved"));
    stdio::kprintf("CPU - eip: %x - cs: %x - ss: %x - ebp: %x - esp: %x\n", r->eip, r->cs, r->ss, r->ebp, r->esp);
    __asm__ volatile ("cli; hlt");  // Halt the cpu Completely hangs the computer
    return;
}

extern "C" void irq_handler(registers_t* r) { // PIC - IRQs Handler
    // stdio::kprintf("IRQ(%d) - IRQ_CODE(%d)\n", r->int_no, r->err_code);
    
    if (interruptHandlers[r->int_no] != 0) {
        interruptHandlers[r->int_no](r); // If the handler is not null notify the handler about the interruption
    }

    pic::sendEOI(r->err_code & 0xFF); // Send the EOI End Of Interruption signal to the PIC IRQ line that was triggered
    return;
}

extern "C" void isr48_handler(IntRegisters r) { // USER - ISR Handler
    asm("mov %0, %%esp" : : "r" (kernelESP)); // Change context to the kernel stack pointer
    PID runningProcess = scheduler::getRunningProcess();
    scheduler::processSaveContext(runningProcess, &r);
    syscalls::syscallHandler(&r);
    // Switch to process context to keep it's execution from where it stopped
    scheduler::processLoadContext(runningProcess);
}

void isr::install() {
    __asm__ __volatile__("cli");  // Clear the interrupt flag in flags CPU Register
    uint8_t i;

    // Setup the isr interrupt functions that was created in isr_int.asm
    for (i = 0; i < 32; i++) {
        idt::setGate(i, (uint32_t) isr_stub_table[i]);
    }

    // Remap Master PIC to offset 0x20 = IDT 32
    // Remap Slave  PIC to offset 0x28 = IDT 40
    pic::remap(0x20, 0x28);

    // Setup the irq interrupt functions that was created in isr_int.asm
    for (; i<48; i++) {
        idt::setGate(i, (uint32_t) isr_stub_table[i]);
    }
    
    // Setup the user interrupt functions that was created in isr_int.asm
    for (; i<49; i++) {
        idt::setGate(i, (uint32_t) isr_stub_table[i]);
    }

    idt::install();
    __asm__ __volatile__("sti");  // Set the interrupt flag in flags CPU Register
}

void isr::registerIsrHandler(uint8_t isrIndex, isr_t handler) {
    interruptHandlers[isrIndex] = handler;
}