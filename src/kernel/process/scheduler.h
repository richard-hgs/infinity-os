#pragma once
#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
// libc
#include <stdint.h>
// memory
#include "heap.h"

// Process state
#define PROC_STATE_NEW 1
#define PROC_STATE_RUNNING 2
#define PROC_STATE_WAITING 3
#define PROC_STATE_READY 4

// Process priority
#define PROC_PRIORITY_SYSTEM 0
#define PROC_PRIORITY_USER 1

#define PROC_UNUSED_PAGE 0xFFFFFFFF

// Max memory pages that can be alloc for one process
#define PROC_MAX_MEMORY_PAGES 20

typedef struct {
    unsigned int EAX, EBX, ECX, EDX, ESP, EBP, ESI, EDI; // general registers
    unsigned int EFLAGS;                                 // flags registers
    unsigned int EIP;                                    // instruction pointer register
    unsigned int CS, SS, ES, DS, FS, GS;                 // segment registers
} __attribute__((packed)) SchedulerRegs;

typedef struct {
    char processName[32];                               // Process name
    unsigned char processState;                         // Process state
    unsigned int pid;                                   // Process id
    unsigned char priority;                             // Process priority
    SchedulerRegs registers;                            // Process context state
    unsigned int memoryPages[PROC_MAX_MEMORY_PAGES];    // Addresses of process memory pages
    Heap processHeap;                                   // User process heap
} PCB;

typedef PCB* PID;

namespace scheduler {
    /**
     * @brief Initialize process scheduler
     *        Initialize allProcesses queue
     *        Initialize readyProcesses queue
     *        Initialize waitingProcesses queue
     */
    void init();

    /**
     * @brief 
     * 
     * @param pages 
     * @param processName 
     * @return unsigned int 
     */
    unsigned int loadProcess(unsigned int *pages, const char* processName);

    /**
     * @brief Create a new process
     * 
     */
    PID createProcess(const char* processName);

    /**
     * @brief Resume the given Process Control Block
     * 
     * @param pid PCB* Process Control Block
     */
    void resumeProcess(PID pid);

    /**
     * @brief Load the register with given Process Control Block information
     * 
     * @param pid PCB* Process Control Block
     */
    void processLoadContext(PID pid);
}

#endif