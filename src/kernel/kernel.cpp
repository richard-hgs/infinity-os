#include <stdint.h>
// legacy drivers
#include "vga.h"
// stdlibs
#include "stdio.h"
// cpu
#include "gdt.h"
#include "isr.h"
#include "paging.h"
// memory
#include "kheap.h"
#include "kernel.h"

// extern "C" void _start();


extern "C" int kmain()
{
    // Clear VGA screen
    vga::clearScreen();

    // Kernel entry point in memory
    // stdio::kprintf("KERNEL_ENTRY: %x\n", (int) _start);

    // Install a new GDT table
    gdt::install();
    vga::printStr("GDT         - Install: OK\n");

    // Install ISR and IDT tables
    isr::install();
    vga::printStr("IDT and ISR - Install: OK\n");

    // Install HEAP - Kernel Heap
    // kheap::install();
    // vga::printStr("HEAP - Install: OK\n");

    // Install MMU - Paging tables
    paging::install();
    vga::printStr("MMU Paging  - Install: OK\n");

    paging::test();

    // Throw an exception to test IDT ISR
    // __asm__ ("mov %eax, %0" :: "r"(1));
    // Division by zero
	// __asm__ ("div %0" :: "r"(0));

    // Dont consume cpu
    __asm__ volatile ("cli; hlt");  // Halt the cpu Completely hangs the computer

    // Idle process consumes cpu
    // while(1) {}

    return 0;
}