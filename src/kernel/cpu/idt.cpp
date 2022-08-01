// stdlibs
#include "bitwise.h"
#include "stdio.h"
#include "idt.h"

idt_gate_t mIdt[IDT_ENTRIES];
idt_reg_t idt_reg;

void idt::setGate(uint16_t i, uint32_t handler, uint16_t cs, uint8_t present, uint8_t dpl, uint8_t storageSegment, uint8_t gateType) {
	mIdt[i].low_offset = low_16(handler);   																			// Get the low 16 bits from idt function ptr
	mIdt[i].sel = cs;                																			        // Segment Selector should point to GDT Kernel Code Segment
	mIdt[i].always0 = 0;                    																			// Bits 32-39 = 00000000. The low 5 bits are reserved, and the high 3 bits are always 000
	mIdt[i].flags = ((present & 0x1) << 7) | ((dpl & 0x3) << 5) | ((storageSegment & 0x1) << 4) | (gateType & 0xF);     // Flags 0x8E = 10001110 [7 = Present Segment is valid Bit 1, 6-5 = Privilege level (00 Most privileged since GDT kernel CS is 00), 4 = Storage segment, must be 0 for interruption gates, 3 = Bit 1 = 32 bits, 2-0 = Bits are always 110]
	mIdt[i].high_offset = high_16(handler); 																			// Get the high 16 bits from IDT function ptr
}

void idt::install(void) {
	idt_reg.limit = IDT_ENTRIES * sizeof(idt_gate_t) - 1;
	idt_reg.base = (uint32_t) &mIdt;

    // Load the idt base configuration into CPU system
	__asm__ __volatile__("lidtl (%0)" : : "r"(&idt_reg));
}