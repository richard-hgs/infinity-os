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
     */
    void printStr(const char* str);
}

#endif