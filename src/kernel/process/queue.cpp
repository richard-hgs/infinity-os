// stdlibs
#include "stdlib.h"
// memory
#include "heap.h"
#include "queue.h"

void queue::init(Queue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
}

int queue::add(Queue *queue, void *data) {
    QueueElement_t *element;

    element = (QueueElement_t *) heap::kmalloc(sizeof(QueueElement_t));
    if (element == NULL) {
        return 0; // failure
    }

    element->data = data;
    element->next = NULL;

    if (queue->front == NULL) {      // Queue is empty so:
        queue->front = element;      // The first queue element is this new element being added.
        queue->rear = element;       // Also the last queue element is this new element being added.
    } else {                         // Queue contains elements so:
        queue->rear->next = element; // Adds a reference to this new rear element in the last queue element.
        queue->rear = element;       // Substitute the last queue element by this new element.
    }

    return 1; // success
}

void *queue::removeFirst(Queue *queue) {
    QueueElement_t *temp;
    void *data;

    temp = queue->front;
    if (temp == NULL) {                // Queue is empty so we cant remove or return the data.
        return NULL;
    }
    queue->front = queue->front->next; // Replace the first queue element by the next element on the queue. 
    data = temp->data;                 // Get the data stored in the first queue element being removed.
    heap::kfree(temp);                 // Release dynamic allocated memory of this queue element in the moment it was added to the queue.

    if (queue->front == NULL) {        // If no next queue element found.
        queue->rear = NULL;            // Nullify the last element, since the queue now is empty.
    }

    return data;                       // Return the data from the first queue element removed from the queue.
}

bool queue::removeElement(Queue *queue, void *data) {
    QueueElement_t *temp, *prev;

    temp = queue->front;                // Save first queue element in temp var.
    if (temp == NULL) {                 // Queue is empty we can't remove a element that didn't exist.
        return false;
    }

    if (temp->data == data) {           // First queue element matches the data being removed from the queue.
        removeFirst(queue);             // So we remove this data from the first queue element.
        return true;
    }

    while (temp != NULL) {              // While element to remove not found.
        if (temp->data == data) {       // Element to remove found.
            prev->next = temp->next;    // Assign the next element after the current element as the new next element of the queue since current element is being removed.
            if (prev->next == NULL) {   // Element being removed is the last element.
                queue->rear = prev;     // So we assing the previous queue element as the new last queue element.
            }
            heap::kfree(temp);          // Release dynamic memory allocated to this queue element.
            return true;                // Return that element was found
        }
        prev = temp;                    // Save current queue element to be compared with next queue element.
        temp = temp->next;              // Save the next queue element to current queue element being compared.
    }

    return false;                       // Element not found in queue.
}

int queue::size(Queue *queue) {
    QueueElement_t *element;
    int size = 0;

    element = queue->front;
    while (element != NULL) {
        element = element->next;
        size++;
    }

    return size;
}