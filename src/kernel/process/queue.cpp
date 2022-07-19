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

    element = (QueueElement_t *)heap::kmalloc(sizeof(QueueElement_t));
    if (element == NULL) {
        return 0; // failure
    }

    element->data = data;
    element->next = NULL;

    if (queue->front == NULL) {
        queue->front = element;
        queue->rear = element;
    } else {
        queue->rear->next = element;
        queue->rear = element;
    }

    return 1; // success
}

bool queue::removeElement(Queue *queue, void *data) {
    QueueElement_t *temp, *prev;

    temp = queue->front;
    if (temp == NULL) { // Queue is empty
        return false;
    }

    if (temp->data == data) { // Queue element to remove found
        removeFirst(queue);
        return true;
    }

    while (temp != NULL) { // While element to remove not found
        if (temp->data == data) { // Element to remove found.
            prev->next = temp->next;
            if (prev->next == NULL) { // Element being removed is the last element.
                queue->rear = prev;
            }
            heap::kfree(temp);
            return true;
        }
        prev = temp;
        temp = temp->next;
    }

    return false;
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