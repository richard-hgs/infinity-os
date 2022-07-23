// stdlibs
#include "stdlib.h"
#include "string.h"
#include "stdio.h" // Debug only
// cpu
#include "paging.h"
// memory
#include "heap.h"
#include "stack.h"
// process
#include "queue.h"
// sys
#include "fs.h"
#include "scheduler.h"

Queue allProcesses;
Queue readyProcesses;
Queue waitingProcesses;
PID runningProcess;

Stack waitingKeyboardProcesses;

PID idleProcess;

unsigned int kernelESP;

void scheduler::init() {
    queue::init(&allProcesses);
    queue::init(&readyProcesses);
    queue::init(&waitingProcesses);
}

unsigned int scheduler::loadProcess(unsigned int *pages, const char* processName) {
    const FileNode* program;
    int pageCount;
    int i;
    char *programText;
    unsigned int bytesToCopy;
    unsigned int bytesCopied = 0;

    program = fs::findFile(processName);
    if (program == NULL) {
        return 0; // File not found
    }

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

    // Initializing process stack
    pcb->memoryPages[PROC_MAX_MEMORY_PAGES - stackOffet] = paging::frameAddress(paging::frameAlloc());

    // stdio::kprintf("%s - (%d) - STACK: 0x%x\n", processName, PROC_MAX_MEMORY_PAGES - stackOffet, pcb->memoryPages[PROC_MAX_MEMORY_PAGES - stackOffet]);

    // Initializing heap
    for (i = progPageCount; i < PROC_MAX_MEMORY_PAGES - heapOffset; i++) {
        pcb->memoryPages[i] = paging::frameAddress(paging::frameAlloc());
    }
    // stdio::kprintf("%s - (%d) - HEAP: 0x%x - 0x%x\n", processName, PROC_MAX_MEMORY_PAGES - heapOffset, pcb->memoryPages[progPageCount], pcb->memoryPages[PROC_MAX_MEMORY_PAGES - heapOffset - 1]);

    // heap::init(&pcb->processHeap, progPageCount * FRAME_SIZE, (i - progPageCount));

    // Initializing registers
    pcb->registers.EAX = 0;
    pcb->registers.EBX = 0;
    pcb->registers.ECX = 0;
    pcb->registers.EDX = 0;
    pcb->registers.ESP = (PROC_MAX_MEMORY_PAGES - espOffset) * FRAME_SIZE - 4;
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
    runningProcess = pcb;

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
    for (i = 0; i < PROC_MAX_MEMORY_PAGES; i++) {
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

PID scheduler::getRunningProcess() {
    return runningProcess;
}