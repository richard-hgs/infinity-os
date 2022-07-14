#include "vga.h"
#include "syscalls.h"

void syscalls::syscallHandler(IntRegisters* r) {
    vga::printStr("ISR(48 - 0x30) - Handler\n");
}