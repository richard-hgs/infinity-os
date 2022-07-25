#include "io.h"

uint8_t io::inb(uint16_t port) {
	uint8_t value;
	__asm__ __volatile__("in %%dx, %%al" : "=a"(value) : "d"(port));
	return value;
}

void io::outb(uint16_t port, uint8_t value) {
	__asm__ __volatile__("out %%al, %%dx" : : "a"(value), "d"(port));
}

uint16_t io::inw(uint16_t port) {
	uint16_t value;
	__asm__ __volatile__("in %%dx, %%ax" : "=a"(value) : "d"(port));
	return value;
}

void io::outw(uint16_t port, uint16_t value) {
	__asm__ __volatile__("out %%ax, %%dx" : : "a"(value), "d"(port));
}

void io::wait() {
	outb(0x80, 0);
}