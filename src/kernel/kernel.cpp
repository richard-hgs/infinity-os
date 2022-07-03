#include <stdint.h>
// legacy drivers
#include "vga.h"
// stdlibs
#include "stdio.h"
// cpu
#include "gdt.h"
#include "isr.h"
#include "paging.h"
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

    // Install MMU - Paging tables
    paging::install();
    vga::printStr("MMU Paging  - Install: OK\n");

    // Throw an exception to test IDT ISR
    // __asm__ ("mov %eax, %0" :: "r"(1));
    // Division by zero
	__asm__ ("div %0" :: "r"(0));

    // vga::printStr("kernel loaded");

    // const char* myPtr = "abc";

    // stdio::kprintf("ABC %d\n123\n456\n789\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nABC\nDEF\nGHI\nJKL\nMNO\nPQR", (int) myPtr);

    // stdio::kprintf("testando %d\nabc", 1);

    // Idle process
    while(1) {}

    return 0;
}