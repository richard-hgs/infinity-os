// legacy drivers
#include "vga.h"
// stdlibs
#include "stdio.h"
#include "syscalls.h"
#include "scheduler.h"

void syscalls::syscallHandler(IntRegisters* r) {
    stdio::kprintf("ISR(48 - 0x30) - (EAX=0x%x) - (EBX=0x%x) - (ECX=0x%x) - (EDX=0x%x)\n", r->eax, r->ebx, r->ecx, r->edx);
    stdio::kprintf("                 (ESP=0x%x) - (EIP=0x%x) - (ESI=0x%x) - (EDI=0x%x)\n", r->esp, r->eip, r->esi, r->edi);
    if (r->eax == SYSCALL_PRINT) {          // SYSCALL - Print a raw text on screen 
        vga::printStr((char*) r->esi);
    }

    // PID runningProcess = scheduler::getRunningProcess();
    // scheduler::processSaveContext(runningProcess, r);
    // scheduler::processLoadContext(runningProcess);
}