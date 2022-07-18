#pragma once
#ifndef _HEAP_H_
#define _HEAP_H_
// libc
#include <stdint.h>
#include <stdbool.h>

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

/**
 * @brief Heap memory element, free or in use. Define if regions of a heap is used or free
 * 
 */
typedef struct HeapElement {
    bool free;                      // true=Free, false=InUse
    struct HeapElement* next;       // Reference to next heap element
    struct HeapElement* prev;       // Reference to previous heap element
    unsigned int dataSize;          // Size of this heap element
    void* data;                     // Address where data starts
} HeapElement_t;

/**
 * @brief Information about heap, baseAddress, first and last heap elements, heap size, etc
 * 
 */
typedef struct {
    unsigned int baseAddress;       // Address where heap starts
    unsigned int freeMemAddress;    // Address of next free memory available
    unsigned int size;              // Size of heap
    HeapElement_t* head;            // First heap element
    HeapElement_t* tail;            // Last heap element
} Heap;

namespace heap {
    /**
     * @brief Initialize the given heap element with its initial information
     * The base address of the paging heap and its size aligned to paging frame size
     * For instance: 1 frame = 4kb
     * 
     * @param heap          Heap instance to be initialized
     * @param baseAddress   The base address where this heap memory will start
     * @param sizeInFrames  The size of this heap aligned to page frame size
     */
    void init(Heap* heap, unsigned int baseAddress, unsigned int sizeInFrames);

    /**
     * @brief Insert a new HeapElement to given heap instance
     * 
     * @param heap          Heap that will receive a new HeapElement
     * @param heapElement   The HeapElement that will be added to a Heap instance
     */
    void insert(Heap* heap, HeapElement* heapElement);

    /**
     * @brief Removes the last heap element of given heap
     * 
     * @param heap Heap to remove last heap element
     */
    void remove(Heap* heap);

    /**
     * @brief Appends new heap elements to the heap until no free heap memory founds. 
     *        Then search for free heap elements that fits the allocated size.
     *        If no free element is found the return is NULL (In future we need to perform a heap reallocation).
     * 
     * @param heap      Heap instance that will hold the new allocated data.
     * @param size      Size of the data being allocated.
     * @return void*    The pointer reference of the allocated data. or NULL=No free HeapElement found.
     */
    void* malloc(Heap* heap, unsigned int size);
    
    /**
     * @brief Free the heap element that was in use inside the heap. 
     *        Also remove this element from the heap to be reused by some other data.
     * 
     * @param heap  Heap instance that will free one heap element.
     * @param addr  The heap element data address that is being free.
     */
    void free(Heap* heap, void* addr);

    /**
     * @brief Initialize the kernel heap baseAddr and size
     * 
     */
    void initKheap();

    /**
     * @brief Allocate a new data in kernel heap space instance
     * 
     * @param size      Size being allocated.
     * @return void*    The pointer reference of the allocated data. or NULL=No free HeapElement found.
     */
    void* kmalloc(unsigned int size);

    /**
     * @brief Free the heap element that was in use inside the kernel heap instance.
     *        Also remove this element from the kernel heap to be reused by some other data.
     * 
     * @param addr 
     */
    void kfree(void* addr);
}

#endif