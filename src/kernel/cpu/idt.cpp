
#include "bitwise.h"
#include "idt.h"

#define KERNEL_CS 0x08  // 0x08 is a stand-in to our kernel code segment in GDT table
#define IDT_ENTRIES 256

idt_gate_t mIdt[IDT_ENTRIES];
idt_reg_t idt_reg;

void idt::setGate(uint8_t i, uint32_t handler)
{
	mIdt[i].low_offset = low_16(handler);   // Get the low 16 bits from idt function ptr
	mIdt[i].sel = KERNEL_CS;                // Segment Selector should point to GDT Kernel Code Segment
	mIdt[i].always0 = 0;                    // Bits 32-39 = 00000000. The low 5 bits are reserved, and the high 3 bits are always 000
	mIdt[i].flags = 0x8E;                   // Flags 0x8E = 10001110 [7 = Present Segmen is valid Bit 1, 6-5 = Privilege level (00 Most privileged since GDT kernel CS is 00), 4 = Bit always 0, 3 = Bit 1 = 32 bits, 2-0 = Bits are always 110]
	mIdt[i].high_offset = high_16(handler); // Get the high 16 bits from IDT function ptr
}


void idt::install(void)
{
	idt_reg.base = (uint32_t) &mIdt;
	idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;

    // Load the idt base configuration into CPU system
	__asm__ __volatile__("lidtl (%0)" : : "r"(&idt_reg));
}