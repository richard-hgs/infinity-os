#include <stdint.h>
#include "vga.h"
#include "stdio.h"
#include "kernel.h"

extern "C" int kmain()
{
    // Clear VGA screen
    vga::clearScreen();
    // vga::printStr("kernel loaded");

    const char* myPtr = "abc";

    stdio::kprintf("ABC %d\n123\n456\n789\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nABC\nDEF\nGHI\nJKL\nMNO\nPQR", (int) myPtr);

    // stdio::kprintf("testando %d\nabc", 1);

    // Idle process
    while(1) {}

    return 0;
}