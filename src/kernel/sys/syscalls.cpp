// legacy drivers
#include "vga.h"
// stdlibs
#include "stdio.h"
#include "syscalls.h"

void syscalls::syscallHandler(IntRegisters* r) {
    stdio::kprintf("ISR(48 - 0x30) - (EAX=0x%x)\n", r->eax);
    if (r->eax == SYSCALL_PRINT) {
        vga::printStr((char*) r->esi);
    }
    
}