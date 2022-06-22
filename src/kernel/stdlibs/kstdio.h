#pragma once
#ifndef _KSTDIO_H_
#define _KSTDIO_H_

/**
 * @brief Standard kernel input output utilities
 * 
 */
namespace kstdio {
    void kprintf(const char* str, ...);

    void kprintf(int foreColor, int bgColor, const char* str, ...);
}

#endif