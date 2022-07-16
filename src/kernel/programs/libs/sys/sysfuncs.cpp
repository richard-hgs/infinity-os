// sys
#include "syscalls.h"
#include "sysfuncs.h"

void sysfuncs::printStr(const char* str) {
    // Executes the interruption INT=(0x30=48) with EAX=(0x01=1=SYSCALL_PRINT) with ESI=(const char*=text_to_print)
    __asm__ __volatile__ (
        "mov %0, %%eax\n\t"
        "mov %1, %%esi\n\t"
        "int $0x30\n\t" 
        : /* output */ 
        : /* input */ "r"(SYSCALL_PRINT), "r"(str)
        : /* clobbers */ "eax", "esi"
    );
}