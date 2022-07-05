#include "kheap.h"

#define STACK_TOP 0x90000 

extern uint32_t __MAX_ADDR; // Max memory address that is used by kernel variables. Created in link.ld
uint32_t kheap_curMemAddress;

uint32_t kheap::kmalloc_int(uint32_t sz, int align, uint32_t *phys) {
    // This will eventually call malloc() on the kernel heap.
    // For now, though, we just assign memory at kheap_curMemAddress
    // and increment it by sz. Even when we've coded our kernel
    // heap, this will be useful for use before the heap is initialised.
    if (align == 1 && (kheap_curMemAddress & 0xFFFFF000) )
    {
        // Align the placement address;
        kheap_curMemAddress &= 0xFFFFF000;
        kheap_curMemAddress += 0x1000;
    }
    if (phys)
    {
        *phys = kheap_curMemAddress;
    }
    uint32_t tmp = kheap_curMemAddress;
    kheap_curMemAddress += sz;
    return tmp;
}

uint32_t kheap::kmalloc_a(uint32_t sz) {
    return kmalloc_int(sz, 1, 0);
}

uint32_t kheap::kmalloc_p(uint32_t sz, uint32_t *phys) {
    return kmalloc_int(sz, 0, phys);
}

uint32_t kheap::kmalloc_ap(uint32_t sz, uint32_t *phys) {
    return kmalloc_int(sz, 1, phys);
}

uint32_t kheap::kmalloc(uint32_t sz) {
    return kmalloc_int(sz, 0, 0);
}

void kheap::install() {
    kheap_curMemAddress = (uint32_t) &(__MAX_ADDR) + STACK_TOP; // Get the pointer address of this variable wich is the last address of the kernel in memory;
}