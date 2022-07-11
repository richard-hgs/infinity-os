#include <stdint.h>
// legacy drivers
#include "vga.h"
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
#include "kernel.h"

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
    vga::printStr("GDT         - Install: OK\n");

    // Install ISR and IDT tables
    isr::install();
    vga::printStr("IDT and ISR - Install: OK\n");

    // Install APIC
    errorCode = apic::install();
    handleError(errorCode, "APIC");
    vga::printStr("APIC        - Install: OK\n");

    // Install HEAP - Kernel Heap
    // kheap::install();
    // vga::printStr("HEAP - Install: OK\n");

    // Install MMU - Paging tables
    paging::install();
    vga::printStr("MMU Paging  - Install: OK\n");
    // paging::test();

    // uint32_t eax;
    // uint32_t ebx;
    // uint32_t ecx;
    // uint32_t edx;
    // cpuid::getCpuid(1, &eax, &ebx, &ecx, &edx);

    cpuid::printCpuInfo();

    // stdio::kprintf("cpuid - eax: %x - ebx: %x - ecx: %x - edx: %x\n", eax, ebx, ecx, edx);

    // Throw an exception to test IDT ISR
    // __asm__ ("mov %eax, %0" :: "r"(1));
    // Division by zero
	// __asm__ ("div %0" :: "r"(0));

    // Dont consume cpu
    __asm__ volatile ("cli; hlt");  // Halt the cpu. Waits until an IRQ occurs

    // Idle process consumes cpu
    // while(1) {}

    return 0;
}