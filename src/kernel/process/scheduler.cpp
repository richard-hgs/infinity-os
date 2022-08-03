// libc
#include <stdbool.h>
// stdlibs
#include "stdlib.h"
#include "string.h"
#include "stdio.h" // Debug only
// cpu
#include "paging.h"
// memory
#include "heap.h"
#include "stack.h"
#include "memutils.h" // Debug only
// process
#include "queue.h"
// sys
#include "fs.h"
#include "scheduler.h"
#include "syscalls.h"

Queue allProcesses;
Queue readyProcesses;
Queue waitingProcesses;
PID runningProcess;

// Only one process can access a keyboard resource per time.
// Also this resource should be discarded, after it's usage.
Stack waitingKeyboardProcesses;

unsigned int kernelESP;

void scheduler::init() {
    // Global vars are located in .bss section unitialized data. Must be initialized.
    queue::init(&allProcesses);
    queue::init(&readyProcesses);
    queue::init(&waitingProcesses);
    stack::init(&waitingKeyboardProcesses);
    kernelESP = 0;

}

void scheduler::start() {
    runningProcess = (PID) queue::removeFirst(&readyProcesses);
    if (runningProcess == NULL) {
        // No ready processes, stop cpu execution until next interruption to save power consumption.
        // stdio::kprintf("SCHEDULER - no ready process found.\n");
        __asm__ volatile ("sti");       // Enable interruptions again to use hlt.
        while(runningProcess == NULL) { // This is our idle process.
            __asm__ volatile ("hlt");   // Halt the cpu. Waits until an IRQ occurs minimize CPU usage, heat and consumption.
            runningProcess = (PID) queue::removeFirst(&readyProcesses);
        }
        __asm__ volatile ("cli");       // Disable interruptions again since one or more processes are in execution.
    }

    // kprintf("sched: %s\n", runningProcess->processName);

    processLoadContext(runningProcess);
}

unsigned int scheduler::loadProcess(unsigned int *pages, const char* processName) {
    const FileNode* program;
    int pageCount;
    int i;
    char *programText;
    unsigned int bytesToCopy;
    unsigned int bytesCopied = 0;
    unsigned int binMainRetOffset = 0;      // Offset where the main function return instruction is located at program binary data
    unsigned int binMainRetCodeOffset = 0;  // Offset where the main function return code is saved in EAX register at program binary data

    program = fs::findFile(processName);
    if (program == NULL) {
        return 0; // File not found
    }

    // Get amount of pages to be allocated for program code size.
    // Also appended the size of the binary of the syscall exit injected when main function return is reached.
    pageCount = paging::sizeInFrames(program->size);

    for (i = 0; i < pageCount; i++) {
        pages[i] = paging::frameAddress(paging::frameAlloc());
        paging::remoteMapPage(pages[i], pages[i]);
        programText = (char*) pages[i];

        bytesToCopy  = program->size - bytesCopied;
        if (bytesToCopy > FRAME_SIZE) {
            bytesToCopy = FRAME_SIZE;
        }

        for (auto j = 0u; j < bytesToCopy; j++) {
            programText[j] = program->data[i * FRAME_SIZE + j];
        }

        bytesCopied += bytesToCopy;
        paging::unmapPage(pages[i]);

        stdio::kprintf("SCHED - allocatingPages: 0x%x\n", pages[i]);
    }

    stdio::kprintf("SCHED - bytesCopied: %d\n", bytesCopied);

    return pageCount;
}

PID scheduler::createProcess(const char* processName) {
    PCB *pcb;
    int i;
    int progPageCount = 0; // pages for program text

    pcb = (PCB*) heap::kmalloc(sizeof(PCB));

    if (pcb == NULL) {
        return 0;
    }

    string::strcpy(pcb->processName, processName);
    pcb->processState = PROC_STATE_NEW;
    pcb->pid = (unsigned int) pcb;
    pcb->priority = PROC_PRIORITY_USER;

    for (i = 0; i < PROC_MAX_MEMORY_PAGES; i++) {
        pcb->memoryPages[i] = PROC_UNUSED_PAGE;
    }

    progPageCount = loadProcess(pcb->memoryPages, processName); // load program text
    if (progPageCount == 0) {
        return NULL;
    }

    int espOffset = 1;
    int stackOffet = 2;
    int heapOffset = 3;

    /*
        N=Number of pages needed to program executable code in memory.

        PROCESS MEMORY LAYOUT (N=1):
        PROGRAM: 0,
        HEAP: 1-16,
        UNUSED: 17,
        STACK: 18,
        UNUSED(ESP-1_BYTE): 19
    */

    // Initializing process stack (PAGE POSITION 17)
    pcb->memoryPages[PROC_MAX_MEMORY_PAGES - stackOffet] = paging::frameAddress(paging::frameAlloc());

    // stdio::kprintf("%s - (%d) - STACK: 0x%x\n", processName, PROC_MAX_MEMORY_PAGES - stackOffet, pcb->memoryPages[PROC_MAX_MEMORY_PAGES - stackOffet]);

    // Initializing heap (PAGE POSITION 1-16)
    for (i = progPageCount; i < PROC_MAX_MEMORY_PAGES - heapOffset; i++) {
        pcb->memoryPages[i] = paging::frameAddress(paging::frameAlloc());
    }
    // stdio::kprintf("%s - (%d) - HEAP: 0x%x - 0x%x\n", processName, PROC_MAX_MEMORY_PAGES - heapOffset, pcb->memoryPages[progPageCount], pcb->memoryPages[PROC_MAX_MEMORY_PAGES - heapOffset - 1]);

    heap::init(&pcb->processHeap, progPageCount * FRAME_SIZE, (i - progPageCount));

    stdio::kprintf("PAGE_LAYOUT: ");
    for (i=0; i<PROC_MAX_MEMORY_PAGES; i++) {
        stdio::kprintf("%x, ", pcb->memoryPages[i]);
    }
    stdio::kprintf("\n");

    // Initializing registers
    pcb->registers.EAX = 0;
    pcb->registers.EBX = 0;
    pcb->registers.ECX = 0;
    pcb->registers.EDX = 0;
    pcb->registers.ESP = (PROC_MAX_MEMORY_PAGES - espOffset) * FRAME_SIZE - 4; // Decrement 4 to let 1 byte free in end of process memory layout.
    pcb->registers.EBP = 0;
    pcb->registers.ESI = 0;
    pcb->registers.EDI = 0;
    pcb->registers.EFLAGS = 1 << 9; // IT (interrupt flag) set. Enable interruptions
    pcb->registers.EIP = 0;
    pcb->registers.CS = 0x8;        // GDT - code segment 0x8
    pcb->registers.SS = 0x10;       // GDT - data segment 0x10
    pcb->registers.DS = 0x10;
    pcb->registers.ES = 0x10;
    pcb->registers.FS = 0x10;
    pcb->registers.GS = 0x10;

    // stdio::kprintf("%s - ESP: 0x%x\n", processName, pcb->registers.ESP);

    queue::add(&allProcesses, (void*) pcb->pid);

    // Debug only
    // runningProcess = pcb;

    return pcb;
}

void scheduler::resumeProcess(PID pid) {
    queue::add(&readyProcesses, pid);
    pid->processState = PROC_STATE_READY;
}

void scheduler::processLoadContext(PID pid) {
    int i;

    pid->processState = PROC_STATE_RUNNING;

    // memory switch
    for (i=0; i < PROC_MAX_MEMORY_PAGES; i++) {
        if (pid->memoryPages[i] != PROC_UNUSED_PAGE) {
            paging::unmapPage(i * FRAME_SIZE);
            paging::remoteMapPage(i * FRAME_SIZE, pid->memoryPages[i]);
        }
    }

    paging::pagesRefresh();
    //kprintf("load: %s %x\n", pid->processName, pid->pid);

    if (kernelESP == 0) { // When the first context switch is performed we save the Last Kernel ESP 
        // kernelESP = 0x6504FE0;
        int currentEspPtr = 0;
        kernelESP = (unsigned int) &currentEspPtr;
    }

    asm("push %0;"
        "pop %%esp;"
        "push %1;"
        "push %2;"
        "push %3;"
        : /* output*/ 
        : /* input */ "g" (pid->registers.ESP), "g" (pid->registers.EFLAGS), "g" (pid->registers.CS),
                      "g" (pid->registers.EIP)
        );
    asm("push %0;"
        "push %1;"
        "push %2;"
        "push %3;"
        : /* output*/
        : /* input */ "g" (pid->registers.DS), "g" (pid->registers.ES), "g" (pid->registers.FS),
                      "g" (pid->registers.GS)
        );
    asm("push %0;"
        "push %1;"
        "push %2;"
        "push %3;"
        "push %4;"
        "push %5;"
        "push %6;"
        "push %7;"
        : /* output */ 
        : /* input  */ "g" (pid->registers.EAX), "g" (pid->registers.ECX), "g" (pid->registers.EDX),
                       "g" (pid->registers.EBX), "g" (pid->registers.ESP), "g" (pid->registers.EBP),
                       "g" (pid->registers.ESI), "g" (pid->registers.EDI)
        );

    asm("popa;"
        "pop %%gs;"
        "pop %%fs;"
        "pop %%es;"
        "pop %%ds;"
        "iret;"
        : /* output */ : /* input */
        );
}

void scheduler::processSaveContext(PID pid, IntRegisters *regs) {
    pid->registers.EAX = regs->eax;
    pid->registers.EBX = regs->ebx;
    pid->registers.ECX = regs->ecx;
    pid->registers.EDX = regs->edx;
    pid->registers.ESI = regs->esi;
    pid->registers.EDI = regs->edi;
    pid->registers.ESP = regs->esp + 12;
    pid->registers.EBP = regs->ebp;
    pid->registers.EFLAGS = regs->eflags;
    pid->registers.EIP = regs->eip;
    pid->registers.CS = regs->cs;
}

void scheduler::processTerminate(PID pid) {
    int i;

    if (queue::removeElement(&allProcesses, (void*) pid) == false) { // No such process
        return;
    }

    // Freeing memory used by the process
    for (i = 0; i < PROC_MAX_MEMORY_PAGES; i++) {
        if (pid->memoryPages[i] != PROC_UNUSED_PAGE) {
            paging::frameFree(paging::frameNumber(pid->memoryPages[i]));
            paging::unmapPage(i * FRAME_SIZE);
        }
    }

    // Removing PID from all process queues
    while (queue::removeElement(&allProcesses, (void*)pid->pid)){}
    while (queue::removeElement(&readyProcesses, (void*)pid->pid)){}
    while (queue::removeElement(&waitingProcesses, (void*)pid->pid)){}
    while (stack::removeElement(&waitingKeyboardProcesses, (void*)pid->pid)){}

    // Freeing process PCB
    heap::kfree(pid);
}

void scheduler::kbdAskResource(PID pid) {
    pid->processState = PROC_STATE_WAITING;                                      // Move process to waiting state
    while (queue::removeElement(&readyProcesses, (void*) runningProcess->pid)){} // Remove process from ready queue
    queue::add(&waitingProcesses, (void*) runningProcess->pid);                  // Add process to waiting queue
    stack::push(&waitingKeyboardProcesses, (void*) runningProcess->pid);         // Add process to waitingKeyboard queue
}

void scheduler::kbdCreateResource(char* kbdBuffer) {
    PID pid;
    int i;

    // Remove process from waiting queues
    pid = (PID) stack::pop(&waitingKeyboardProcesses);
    if (pid != NULL) {
        while (queue::removeElement(&waitingProcesses, (void*) pid->pid)) {}

        // Copy input buffer to process memory, address is in EDI
        for (i=0; i<PROC_MAX_MEMORY_PAGES; i++) {
            if (pid->memoryPages[i] != PROC_UNUSED_PAGE) {
                paging::remoteMapPage(i * FRAME_SIZE, pid->memoryPages[i]);
            }
        }
        string::strcpy((char*) pid->registers.EDI, kbdBuffer);

        // Add process that request this resource to ready queue
        queue::add(&readyProcesses, (void*) pid->pid);
    }
}

PID scheduler::getRunningProcess() {
    return runningProcess;
}

void scheduler::printProcessList() {
    int i;
    Queue* q = &allProcesses;
    QueueElement *e;
    PCB *pcb;

    stdio::kprintf("---------- Processes ---------\n");
    e = q->front;
    while (e != NULL) {
        pcb = (PCB*) e->data;
        stdio::kprintf("%s (%x)", pcb->processName, (unsigned int) pcb);
        e = e->next;
    }
    stdio::kprintf("-----------------------------\n");
}