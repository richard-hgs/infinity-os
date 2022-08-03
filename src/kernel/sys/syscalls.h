#pragma once
#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "isr.h"


#define SYSCALL_PRINT     1    // Print text on screen equivalent to vga::printStr("text\n");
#define SYSCALL_PROC_EXIT 2    // Called when a proccess finish it's execution. Remove from queue and move to the next proccess;
#define SYSCALL_READLN    3    // Read line from console
#define SYSCALL_MALLOC    4    // Dynamic allocate memory in process heap.
#define SYSCALL_FREE      5    // Dynamic free memory in process heap.
#define SYSCALL_PSLIST    6    // Print process list in terminal.

// #define SYSCALL_MALLOC 103 //allocate memory in process heap
// #define SYSCALL_FREE 104 //free memory in process heap
// #define SYSCALL_EXEC_PROGRAM 105 //execute program
// #define SYSCALL_TERMINATE_PROCESS 106 //terminate running process
// #define SYSCALL_PRINT_PROCESSES 107

namespace syscalls {
    void syscallHandler(IntRegisters* r);
}

#endif