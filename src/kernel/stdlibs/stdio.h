#pragma once
#ifndef _STDIO_H_
#define _STDIO_H_

/**
 * @brief Standard kernel input output utilities
 * 
 */
namespace stdio {
    void kprintf(const char* str, ...);

    void kprintf(int foreColor, int bgColor, const char* str, ...);
}

#endif