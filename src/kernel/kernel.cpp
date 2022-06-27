#include <stdint.h>
// legacy drivers
#include "vga.h"
// stdlibs
#include "stdio.h"
// cpu
#include "gdt.h"
#include "isr.h"
#include "kernel.h"

extern "C" int kmain()
{
    // Clear VGA screen
    vga::clearScreen();

    // Install a new GDT table
    gdt::install();
    vga::printStr("GDT         - Install: OK\n");

    // Install isr and idt tables
    isr::install();
    vga::printStr("IDT and ISR - Install: OK\n");

    // Throw an exception to test IDT ISR
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