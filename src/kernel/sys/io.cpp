#include "io.h"

unsigned char io::inb(unsigned short port)
{
	unsigned char value;
	__asm__ __volatile__("in %%dx, %%al" : "=a"(value) : "d"(port));
	return value;
}

void io::outb(unsigned short port, unsigned char value)
{
	__asm__ __volatile__("out %%al, %%dx" : : "a"(value), "d"(port));
}

unsigned short io::inw(unsigned short port)
{
	unsigned short value;
	__asm__ __volatile__("in %%dx, %%ax" : "=a"(value) : "d"(port));
	return value;
}

void io::outw(unsigned short port, unsigned short value)
{
	__asm__ __volatile__("out %%ax, %%dx" : : "a"(value), "d"(port));
}