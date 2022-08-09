#pragma once
#ifndef _SYSFUNCS_H_
#define _SYSFUNCS_H_

extern "C" void __attribute__((section("._start"))) _start();

namespace sysfuncs {
    /**
     * @brief Prints a raw string text with only the Escape Sequences
     * 
     * Executes the interruption INT=(0x30=48) with EAX=(0x01=1=SYSCALL_PRINT) with ESI=(const char*=text_to_print)
     * 
     *  ESCAPE_SEQUENCES: 
     *    - \n Line feed or new line
     * 
     *  MAX_TEXT_SIZE: 1024 chars long
     * 
     */
    void print(const char *str);

    /**
     * @brief Stop proccess execution and return a code if an error happened
     * 
     * Executes the interruption INT=(0x30=48) with EAX=(0x02=2=SYSCALL_PROC_EXIT) and EBX=(code={0=SUCCESS or ERROR_CODE})
     * 
     * @param code 0=SUCCESS or ERROR_CODE
     */
    void exit(int code);

    /**
     * @brief Prints a formatted string text with unlimited arguments
     * 
     * Executes the interruption INT=(0x30=48) with EAX=(0x01=1=SYSCALL_PRINT) with ESI=(const char*=text_to_print)
     * 
     *  FORMATS_SUPPORTED:
     *    - %s  -> Put the char* text from a argument variable in formatted string.
     *    - %c  -> Put the char from a argument variable in formatted string.
     *    - %d  -> Put the int from a argument variable in formatted string.
     *    - %x  -> Put the int in hex format from a argument variable in formatted string.
     *    - %b  -> Put the int in binary format from a argument variable in formatted string.
     *    - %0d -> Leading zeros for above format types %d, $x, %b;
     * 
     *  ESCAPE_SEQUENCES: 
     *    - \n Line feed or new line.
     * 
     * @param str 
     */
    void printf(const char *str, ...);

    /**
     * @brief Read one line from console terminal input by the keyboard driver. 
     *        Continue process execution only after one line of text received.
     * 
     * Executes the interruption INT=(0x30=48) with EAX=(0x03=3=SYSCALL_READLN) with no parameters(E.g The process scheduler already knows wich process is running)
     * 
     * @param dest OUT - The char* buffer that will hold this keyboard input line.
     */
    void readln(char *dest);

    /**
     * @brief Dynamic allocate memory inside process heap free memory.
     *        If no free memory found returs an address of 0.
     * 
     * @param size          Size to be allocated
     * @return unsigned int 0=No free space available, or ptr address to allocated memory.
     */
    unsigned int malloc(unsigned int size);

    /**
     * @brief Dynamic free memory inside process heap.
     * 
     * @param ptr           Ptr address to be free.
     */
    void free(void* ptr);

    /**
     * @brief Print process list
     * 
     */
    void printProcessList();

    /**
     * @brief Execute a program
     * 
     * @param path  Program path.
     * @param argc  Arguments count to be passed to program.
     * @param argv  Arguments to be passed to program.
     * @return 0=Program not found or not created, or PCB id.
     */
    int execv(const char* path, int argc, char* argv[]);

    /**
     * @brief Clear the vga screen
     * 
     */
    void clearScreen();
}

#endif