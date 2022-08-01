// libc ---------------------------------------
#include <stdarg.h>
// kernel libs --------------------------------
// stdlib
#include "stdlib.h"
// user libs ----------------------------------
// sys
#include "ksysfuncs.h"

// -------------- SYSCALLS ARE DEFINED IN ./src/kernel/sys/syscalls.h ------------------
#define SYSCALL_PRINT 1             // Print text on screen vga printStr("myText\n");
#define SYSCALL_PROC_EXIT 2         // Called when a proccess finish it's execution. Remove from queue and move to the next proccess;
#define SYSCALL_READLN 3            // Process wait until one line from console terminal is returned;

#define PRINTF_STR_BUFFER_SIZE 1024

/**
 * @brief Access the main function of the executable process
 * 
 */
extern "C" int main();

/**
 * @brief This is the entry point of a process. It creates and access all required parameters to execute a process
 * 
 */
void __attribute__((section("._start"))) _start() {
    ksysfuncs::exit(main());
}

void ksysfuncs::printStr(const char* str) { // Executes the interruption INT=(0x30=48) with EAX=(0x01=1=SYSCALL_PRINT) with ESI=(const char*=text_to_print)
    __asm__ __volatile__ (
        "mov %0, %%eax;"
        "mov %1, %%esi;"
        "int $0x30;" 
        : /* output */ 
        : /* input */ "r"(SYSCALL_PRINT), "r"(str)
        : /* clobbers */ "eax", "esi"
    );
}


void ksysfuncs::printf(const char* str, ...) { // Format the string and redirects to printStr func
    char fStr[PRINTF_STR_BUFFER_SIZE];
    va_list list;
    va_start(list, str);
    stdlib::va_stringf(fStr, str, list);
    va_end(list);
    printStr(fStr);
}

void ksysfuncs::readln(char* dest) { // Executes the interruption INT=(0x30=48) with EAX=(0x03=3=SYSCALL_READLN) with no parameters(E.g The process scheduler already knows wich process is running)
    __asm__ __volatile__ (
        "mov %0, %%eax;"
        "mov %1, %%edi;"
        "int $0x30;" 
        : /* output */ 
        : /* input */ "r"(SYSCALL_READLN), "r"(dest)
        : /* clobbers */ "eax", "edi"
    );
}

void ksysfuncs::exit(int code) { // Executes the interruption INT=(0x30=48) with EAX=(0x02=2=SYSCALL_PROC_EXIT) and EBX=(code={0=SUCCESS or ERROR_CODE}) - Remove proccess from queue get the return code and move to the next proccess
    __asm__ __volatile__ (
        "mov %0, %%eax;"
        "mov %1, %%ebx;"
        "int $0x30;" 
        : /* output */ 
        : /* input */ "r"(SYSCALL_PROC_EXIT), "r"(code)
        : /* clobbers */ "eax", "ebx"
    );
}