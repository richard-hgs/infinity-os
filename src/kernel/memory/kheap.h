#pragma once
#ifndef _KHEAP_H_
#define _KHEAP_H_
// libc
#include <stdint.h>

extern uint32_t kheap_curMemAddress;

/**
 * @brief RAM MEMORY MAP
 *  _________________________________________________________________________________________
 * |    START   |    END     |         SIZE        | DESCRIPTION                             |
 * | 0x00000000 | 0x000003FF | 1 KiB               | Real Mode IVT (Interrupt Vector Table)  |
 * | 0x00000400 | 0x000004FF | 256 bytes           | BDA (BIOS data area)                    |
 * | 0x00000500 | 0x00007BFF | almost 30 KiB       | Conventional memory                     |
 * | 0x00007C00 | 0x00007DFF | 512 bytes           | Your OS BootSector                      |
 * | 0x00007E00 | 0x0007FFFF | 480.5 KiB           | Conventional memory                     |
 * | 0x00080000 | 0x0009FFFF | 128 KiB             | EBDA (Extended BIOS Data Area)          |
 * | 0x000A0000 | 0x000BFFFF | 128 KiB             | Video display memory                    |
 * | 0x000C0000 | 0x000C7FFF | 32 KiB (typically)  | Video BIOS                              |
 * | 0x000C8000 | 0x000EFFFF | 160 KiB (typically) | BIOS Expansions                         |
 * | 0x000F0000 | 0x000FFFFF | 64 KiB              | Motherboard BIOS                        |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 */

namespace kheap {
    /**
     * @brief Common allocator used by all other allocators
     * 
     * @param sz 
     * @param align 
     * @param phys 
     * @return uint32_t 
     */
    uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phys);

    /**
     * @brief Page Aligned - Kernel heap memory allocation
     * 
     * @param sz 
     * @return uint32_t 
     */
    uint32_t kmalloc_a(uint32_t sz);

    /**
     * @brief Physical Address - Kernel heap memory allocates a memory and returns a physical address
     * 
     * @param sz 
     * @param phys 
     * @return uint32_t 
     */
    uint32_t kmalloc_p(uint32_t sz, uint32_t *phys);

    /**
     * @brief Page Aligned, Physical Address - Kernel heap memory allocates a memory aligned and returns a physical address
     * 
     * @param sz 
     * @param phys 
     * @return uint32_t 
     */
    uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys);

    /**
     * @brief Default - Kernel heap memory default allocation
     * 
     * @param sz 
     * @return uint32_t 
     */
    uint32_t kmalloc(uint32_t sz);

    /**
     * @brief Initialize the heap configuration before using it in kernel space
     * 
     * @return uint32_t 
     */
    void install();
}

#endif