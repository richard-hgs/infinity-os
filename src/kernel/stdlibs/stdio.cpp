// libc
#include <stdarg.h>
// stdlibs
#include "stdio.h"
#include "stdlib.h"
// drivers
#include "vga.h"

#define KPRINTF_STR_BUFFER_SIZE 2048

void _kprintf(int foreColor, int bgColor, const char *str, va_list list) {
    char formatedStr[KPRINTF_STR_BUFFER_SIZE];
    stdlib::va_stringf(formatedStr, str, list);
    vga::printStr(foreColor, bgColor, formatedStr);
}

void stdio::kprintf(const char *str, ...) {
    va_list list;
    va_start(list, str);
    _kprintf(VGA_DEF_FORECOLOR, VGA_DEF_BGCOLOR, str, list);
    va_end(list);
}

void stdio::kprintf(int foreColor, int bgColor, const char *str, ...) {
    va_list list;
    va_start(list, str);
    _kprintf(foreColor, bgColor, str, list);
    va_end(list);
}