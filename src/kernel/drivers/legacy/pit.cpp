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
    // Setup the handler
    isr::registerIsrHandler(IRQ0, timerInterruptHandler);

    // uint32_t divisor = 1193180 / 1193;
	// uint8_t low = (uint8_t)(divisor & 0xff);
	// uint8_t high = (uint8_t)((divisor >> 8) & 0xff);
	// io::outb(0x43, 0x36);
	// io::outb(0x40, low);
	// io::outb(0x40, high);
}

void pit::enable() {
    pic::clearMask(0); // 0 = IRQ0
}

void pit::disable() {
    pic::setMask(0);   // 0 = IRQ0
}