#pragma once
#ifndef _SYSCALLS_H_
#define _SYSCALLS_H_

#include "isr.h"

// #define SYSCALL_EXIT 100 //exit process

#define SYSCALL_PRINT 1   // output string on screen equivalent to vga::printStr("text\n");

// #define SYSCALL_READLN 102 //read line from console
// #define SYSCALL_MALLOC 103 //allocate memory in process heap
// #define SYSCALL_FREE 104 //free memory in process heap
// #define SYSCALL_EXEC_PROGRAM 105 //execute program
// #define SYSCALL_TERMINATE_PROCESS 106 //terminate running process
// #define SYSCALL_PRINT_PROCESSES 107

namespace syscalls {
    void syscallHandler(IntRegisters* r);
}

#endif