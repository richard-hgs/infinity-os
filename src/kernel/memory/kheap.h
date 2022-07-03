#pragma once
#ifndef _KHEAP_H_
#define _KHEAP_H_
// libc
#include <stdint.h>

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
}

#endif