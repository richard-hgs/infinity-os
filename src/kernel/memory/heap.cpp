// stdlibs
#include "stdlib.h"
// cpu
#include "paging.h"
#include "heap.h"

/**
 * @brief Reference to the kernel heap
 * Since the kernel has only one heap we can declare the kernel heap as static
 */
Heap kernelHeap;

void heap::init(Heap* heap, unsigned int baseAddress, unsigned int sizeInFrames) {
    heap->baseAddress = baseAddress;
    heap->freeMemAddress = heap->baseAddress;
    heap->size = sizeInFrames * FRAME_SIZE;
    heap->head = NULL;
    heap->tail = NULL;
}

void heap::insert(Heap* heap, HeapElement_t* heapElement) {
    if (heap->head == NULL) {               // Heap is empty. Set the first element as the head and tail.
        heapElement->next = NULL;
        heapElement->prev = NULL;
        heap->head = heapElement;
        heap->tail = heapElement;
    } else {                                // Heap is not empty. Set the current heap tail as the previous heap element of the new heap element. Set in the last heap element a reference to the new next heap element. Also set this new element as the tail of the heap.
        heapElement->prev = heap->tail;
        heap->tail->next = heapElement;
        heap->tail = heapElement;
    }
}

void heap::remove(Heap* heap) { // Removes last element
    if (heap->tail == heap->head) { // One element left. Since this is the last heap element we cleared entire heap so we need to nullify the head and tail.
        heap->head = NULL;
        heap->tail = NULL;
    } else {                        // More than one element left. Set the previous heap element as the last element of the heap. Also remove the reference of the removed element from previous heap element.
        heap->tail = heap->tail->prev;
        heap->tail->next = NULL;
    }
}

void* heap::malloc(Heap* heap, unsigned int size) {
    int physicalSize;
    HeapElement_t* heapElement;

    physicalSize = size + sizeof(HeapElement_t);
    if (heap->freeMemAddress + physicalSize <= heap->baseAddress + heap->size) { // Memory is allocated at the end of the heap.
        heapElement = (HeapElement_t*) heap->freeMemAddress;
        heapElement->free = false;
        heapElement->dataSize = size;
        heapElement->data = (void*)((int)heapElement + sizeof(HeapElement_t));

        insert(heap, heapElement);
        heap->freeMemAddress += physicalSize;

        return heapElement->data;
    } else { // Since entire heap region is in use we have now to search for a free memory that is big enough.
        heapElement = heap->head;
        while (heapElement != NULL) {
            if ((heapElement->free == true) && (heapElement->dataSize >= size)) {
                heapElement->free = false;
                return heapElement->data;
            } else {
                heapElement = heapElement->next;
            }
        }
    } // If no free element found we need to reallocate heap in the future to prevent malloc nullptrs

    return NULL; // No free memory found
}

void heap::free(Heap* heap, void* addr) {
    HeapElement_t* heapElement;

    if (((unsigned int) addr < heap->baseAddress) || ((unsigned int) addr > (heap->baseAddress + heap->size * FRAME_SIZE))) { // Is address in heap space?
        return;
    }

    heapElement = (HeapElement_t*)((int) addr - sizeof(HeapElement_t)); // Retrieve HeapElement address by subtracting it's size from data address.

    if (heapElement == heap->tail) { // Freeing the last element
        remove(heap);
        heap->freeMemAddress = (unsigned int) heapElement;
    } else {
        heapElement->free = true;
    }
}

void heap::initKheap() {
    init(&kernelHeap, KERNEL_HEAP_START_ADDR, KERNEL_HEAP_SIZE);
}

void* heap::kmalloc(unsigned int size) {
    return malloc(&kernelHeap, size);
}

void heap::kfree(void* addr) {
    free(&kernelHeap, addr);
}