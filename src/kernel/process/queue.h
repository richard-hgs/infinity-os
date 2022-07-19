#pragma once
#ifndef _QUEUE_H_
#define _QUEUE_H_
// libc
#include <stdbool.h>

typedef struct QueueElement {
    void* data;                 // Pointer reference to element data
    struct QueueElement* next;  // Next element
} QueueElement_t;

typedef struct {
    QueueElement_t* front;  // First element
    QueueElement_t* rear;   // Last element
} Queue;

namespace queue {

    /**
     * @brief Initialize the given queue.
     * 
     * @param queue Queue to be initialized
     */
    void init(Queue* queue);

    /**
     * @brief Adds a element to the queue
     * 
     * @param queue Queue that will receive the element
     * @param data  Element data pointer reference
     * @return int  0=Failure, 1=Success
     */
    int add(Queue* queue, void* data);

    /**
     * @brief Remove the first element of the queue and get the pointer reference to its data
     * @return void* pointer data
     */
    void* removeFirst(Queue* queue);

    /**
     * @brief Remove a element from the queue using the pointer reference to its data
     * 
     * @param queue Queue that will get some element removed
     * @param data  Data pointer reference that will be removed
     * @return true   Element removed
     * @return false  Element not removed, maybe not found
     */
    bool removeElement(Queue* queue, void* data);

    /**
     * @brief Return the size(Number of elements) in the specified queue
     * 
     * @param queue The queue to get size of
     * @return int  The amount of elements found
     */
    int size(Queue *queue);
}

#endif