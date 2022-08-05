// libc
#include <stdbool.h>
// legacy drivers
#include "vga.h"
// stdlibs
#include "stdio.h"
#include "syscalls.h"
#include "scheduler.h"

void syscalls::syscallHandler(IntRegisters* r) {
    // Get current running process and save his context
    PID pid = scheduler::getRunningProcess();
    scheduler::processSaveContext(pid, r);
    bool resumeProcess = true;

    // stdio::kprintf("ISR(48 - 0x30) - (EAX=0x%x) - (EBX=0x%x) - (ECX=0x%x) - (EDX=0x%x)\n", r->eax, r->ebx, r->ecx, r->edx);
    // stdio::kprintf("                 (ESP=0x%x) - (EIP=0x%x) - (ESI=0x%x) - (EDI=0x%x)\n", r->esp, r->eip, r->esi, r->edi);
    if (r->eax == SYSCALL_PRINT) {              // SYSCALL - Print a raw text on screen 

        // stdio::kprintf("%s (0x%x) - %s", pid->processName, pid->pid, (char*) pid->registers.ESI);
        vga::printStr((char*) pid->registers.ESI);

    } else if (r->eax == SYSCALL_PROC_EXIT) {   // SYSCALL - Proccess finished it's execution

        if (r->ebx > 0) { // Process finished with error code
            stdio::kprintf("\n%s (0x%x) - Finished with code %d\n", pid->processName, pid->pid, pid->registers.EBX);
        }
        // Terminate this process
        scheduler::processTerminate(pid);
        resumeProcess = false;                  // Process is being terminated so we can't resume its execution.

    } else if (r->eax == SYSCALL_READLN) {      // SYSCALL - Process wants to receive one input line from keyboad.

        scheduler::kbdAskResource(pid);
        resumeProcess = false;                  // Process is waiting for keyboard resource to resume its execution.

    } else if (r->eax == SYSCALL_MALLOC) {      // SYSCALL - Dynamic allocate memory in process heap space.

        pid->registers.EAX = (unsigned int) heap::malloc(&pid->processHeap, pid->registers.EBX);

    } else if (r->eax == SYSCALL_FREE) {         // SYSCALL - Dynamic free memory in process heap space.

        heap::free(&pid->processHeap, (void*) pid->registers.EBX);

    } else if (r->eax == SYSCALL_PSLIST) {      // SYSCALL -  Print process list in terminal.

        scheduler::printProcessList();
    } else if (r->eax == SYSCALL_CLEAR_SCREEN) { // SYSCALL -  Clear vga screen and set cursor at col:0, row:0.
        
        vga::clearScreen();
    }

    if (resumeProcess) {
        // Add this process to the ready queue again to continue its execution.
        scheduler::resumeProcess(pid);
    }

    scheduler::start();
}