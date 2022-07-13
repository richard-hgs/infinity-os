// cpu
#include "isr.h"
// stdio
#include "stdio.h"
#include "keyboard.h"

void keyboardInterruptHandler(registers_t* r) {
    stdio::kprintf("keyboard\n");
}

void keyboard::install() {
    isr::registerIsrHandler(IRQ1, keyboardInterruptHandler);
}