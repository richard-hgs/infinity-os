// cpu
#include "pic.h"
#include "isr.h"
// stdio
#include "stdio.h"
#include "pit.h"

void timerInterruptHandler(registers_t* r) {
    // On each tick exchange the user process
}

void pit::install() {
    isr::registerIsrHandler(IRQ0, timerInterruptHandler);
}

void pit::enable() {
    pic::clearMask(0); // 0 = IRQ0
}

void pit::disable() {
    pic::setMask(0);   // 0 = IRQ0
}