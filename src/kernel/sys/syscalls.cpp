// legacy drivers
#include "vga.h"
// stdlibs
#include "stdio.h"
#include "syscalls.h"
#include "scheduler.h"

void syscalls::syscallHandler(IntRegisters* r) {
    stdio::kprintf("ISR(48 - 0x30) - (EAX=0x%x) - (ESP=0x%x) - (EIP=0x%x) - (SI=0x%x)\n", r->eax, r->esp, r->eip, r->esi);
    if (r->eax == SYSCALL_PRINT) {          // SYSCALL - Print a raw text on screen 
        vga::printStr((char*) r->esi);
    }

    PID runningProcess = scheduler::getRunningProcess();
    scheduler::processSaveContext(runningProcess, r);
    scheduler::processLoadContext(runningProcess);
}