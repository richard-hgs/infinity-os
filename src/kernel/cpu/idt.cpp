
#include "bitwise.h"
#include "idt.h"

// #define KERNEL_CS 0x08
// #define IDT_ENTRIES 256

// idt_gate_t idt[IDT_ENTRIES];
// idt_reg_t idt_reg;

// /* Set the IDT handlers one at a time.
//  */
// void set_idt_gate(int n, uint32_t handler)
// {
// 	idt[n].low_offset = low_16(handler);
// 	idt[n].sel = KERNEL_CS;
// 	idt[n].always0 = 0;
// 	idt[n].flags = 0x8E;
// 	idt[n].high_offset = high_16(handler);
// }
// /* Loads the IDT into system.
//  */
// void set_idt(void)
// {
// 	idt_reg.base = (uint32_t)&idt;
// 	idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
// 	__asm__ __volatile__("lidtl (%0)" : : "r"(&idt_reg));
// }