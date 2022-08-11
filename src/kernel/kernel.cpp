#include <stdint.h>
// legacy drivers
#include "vga.h"
#include "ps2.h"
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
#include "heap.h"
// sys
#include "io.h"
#include "fs.h"
// scheduler
#include "scheduler.h"
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

extern "C" int kmain() {
    uint8_t errorCode;

    const char* OK_MSG = "OK";
    const char* ERR_MSG = "Failed with error code";
    const char* PS2_INSTALL_MSG = "PS/2 Controller - Install:";

    // Clear VGA screen
    vga::clearScreen();

    // Kernel entry point in memory
    // stdio::kprintf("KERNEL_ENTRY: %x\n", (int) _start);

    // Install a new GDT table
    gdt::install();
    stdio::kprintf("GDT             - Install: %s\n", OK_MSG);

    // Install ISR and IDT tables
    // Remap the PIC offsets into the IDT table 32 and 40 and setup the IRQs
    isr::install();
    stdio::kprintf("IDT, ISR, IRQ   - Install: %s\n", OK_MSG);

    // Install APIC
    // errorCode = apic::install();
    // handleError(errorCode, "APIC");
    // vga::printStr("APIC        - Install: OK\n");

    // Install HEAP - Kernel Heap
    // kheap::install();
    // vga::printStr("HEAP - Install: OK\n");

    // Install FS - File System
    fs::install();
    stdio::kprintf("FS File System  - Install: %s\n", OK_MSG);

    // Install MMU - Paging tables
    paging::install();
    stdio::kprintf("MMU Paging      - Install: %s\n", OK_MSG);
    // paging::test();

    // Install PIT - Programmable Interval Timer
    pit::install();
    stdio::kprintf("PIT Timer       - Install: %s\n", OK_MSG);

    // Install PS/2 - Controller
    errorCode = ps2::install();
    if (errorCode == PS2_NO_ERROR) {
        stdio::kprintf("%s %s\n", PS2_INSTALL_MSG, OK_MSG);
    } else {
        stdio::kprintf("%s %s %d\n", PS2_INSTALL_MSG, ERR_MSG, errorCode);
    }
    
    // Install HEAP
    heap::initKheap();
    vga::printStr("KERNEL HEAP     - Install: OK\n");

    scheduler::init();
    PID pidShell = scheduler::createProcess("shell.exe");
    scheduler::resumeProcess(pidShell);
    scheduler::start();

    // scheduler::processLoadContext(pidShell);

    // stdio::kprintf("pidShell 0x%x\n", (int) pidShell);

    // Test interruption
    // sysfuncs::printStr("testando123456\n");

    // cpuid::printCpuInfo();


    // stdio::kprintf("cpuid - eax: %x - ebx: %x - ecx: %x - edx: %x\n", eax, ebx, ecx, edx);

    // Throw an exception to test IDT ISR
    // __asm__ ("mov %eax, %0" :: "r"(1));
    // Division by zero
	// __asm__ ("div %0" :: "r"(0));

    // Dont consume cpu
    // __asm__ volatile ("cli; hlt");  // Halt the cpu. Waits until an IRQ occurs

    

    // Idle process consumes cpu
    while(1) {
        __asm__ volatile ("hlt"); // Halt the cpu. Waits until an IRQ occurs minimize CPU usage
    }

    return 0;
}