#include <stdint.h>
#include "idt.h"
#include "stdio.h" // Debug only
#include "isr.h"

extern "C" void* isr_stub_table[];

extern "C" void isr_handler() {
    stdio::kprintf("ISR - Interruption called;\n");
    __asm__ volatile ("cli; hlt");  // Completely hangs the computer
}

void isr::install() {
    // idt::setGate();
    for (uint8_t i = 0; i < 32; i++) {
        idt::setGate(i, (uint32_t) isr_stub_table[i]);
        // vectors[vector] = true;
    }
    idt::install();

    // stdio::kprintf("isr_stub_table_size: %d\n", isr_stub_table[0]);
}

