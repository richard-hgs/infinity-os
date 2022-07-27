#pragma once
#ifndef _SYSFUNCS_H_
#define _SYSFUNCS_H_

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
    void printStr(const char* str);

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
    void printf(const char* str, ...);

    /**
     * @brief Stop proccess execution and return a code if an error happened
     * 
     * Executes the interruption INT=(0x30=48) with EAX=(0x02=2=SYSCALL_PROC_EXIT) and EBX=(code={0=SUCCESS or ERROR_CODE})
     * 
     * @param code 0=SUCCESS or ERROR_CODE
     */
    void exit(int code);
}

#endif