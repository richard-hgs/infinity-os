#include <stdint.h>
// legacy drivers
#include "vga.h"
#include "keyboard.h"
#include "pit.h"
// stdlibs
#include "stdio.h"
// cpu
#include "gdt.h"
#include "isr.h"
#include "paging.h"
#include "apic.h"
#include "cpuid.h"
// memory
#include "kheap.h"
// sys
#include "io.h"
#include "kernel.h"

extern "C" void gen_interrupt(int index);

// extern "C" void _start();

void handleError(uint8_t errorCode, const char* errorPrefix) {
    if (errorCode > 0) {
        // Some error happend
        stdio::kprintf("%s - ERROR: %d\n", errorPrefix, errorCode);
        __asm__ volatile ("cli; hlt");  // Halt the cpu. Waits until an IRQ occurs
    }
}

extern "C" int kmain()
{
    uint8_t errorCode;

    // Clear VGA screen
    vga::clearScreen();

    // Kernel entry point in memory
    // stdio::kprintf("KERNEL_ENTRY: %x\n", (int) _start);

    // Install a new GDT table
    gdt::install();
    vga::printStr("GDT            - Install: OK\n");

    // Install ISR and IDT tables
    // Remap the PIC offsets into the IDT table 32 and 40 and setup the IRQs
    isr::install();
    vga::printStr("IDT, ISR, IRQ  - Install: OK\n");

    // Install APIC
    // errorCode = apic::install();
    // handleError(errorCode, "APIC");
    // vga::printStr("APIC        - Install: OK\n");

    // Install HEAP - Kernel Heap
    // kheap::install();
    // vga::printStr("HEAP - Install: OK\n");

    // Install MMU - Paging tables
    // paging::install();
    // vga::printStr("MMU Paging    - Install: OK\n");
    // paging::test();

    // Install PIT - Programmable Interval Timer
    pit::install();
    vga::printStr("PIT Timer      - Install: OK\n");

    // Install PS/2 - Keyboard
    keyboard::install();
    vga::printStr("PS/2 Keyboard  - Install: OK\n");

    // uint32_t divisor = 1193180 / 1193;
	// uint8_t low = (uint8_t)(divisor & 0xff);
	// uint8_t high = (uint8_t)((divisor >> 8) & 0xff);
	// io::outb(0x43, 0x36);
	// io::outb(0x40, low);
	// io::outb(0x40, high);
    
    // Test interruption
    __asm__ __volatile__("int $0x30");

    // vga::printStr("teste\n");

    // __asm__ __volatile__("int $0x30");

    // cpuid::printCpuInfo();


    // stdio::kprintf("cpuid - eax: %x - ebx: %x - ecx: %x - edx: %x\n", eax, ebx, ecx, edx);

    // Throw an exception to test IDT ISR
    // __asm__ ("mov %eax, %0" :: "r"(1));
    // Division by zero
	// __asm__ ("div %0" :: "r"(0));

    // Dont consume cpu
    // __asm__ volatile ("cli; hlt");  // Halt the cpu. Waits until an IRQ occurs

    // Idle process consumes cpu
    while(1) {}

    return 0;
}