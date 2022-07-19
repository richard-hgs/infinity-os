// stdlibs
#include "stdlib.h"
#include "stdio.h"
// memory
#include "heap.h"
#include "stack.h"

void stack::init(Stack *s) {
    s->head = NULL;
}

int stack::push(Stack *s, void *data) {
    StackElement_t *stackE;

    stackE = (StackElement_t *) heap::kmalloc(sizeof(StackElement_t));
    if (stackE == NULL) {
        return 0; // Failure
    }

    stackE->data = data;
    stackE->next = s->head;
    s->head = stackE;

    return 1; // Success
}

void *stack::pop(Stack *s) {
    void *data;
    StackElement_t *temp;

    if (s->head == NULL) {
        return NULL;
    }

    data = s->head->data;
    temp = s->head;
    s->head = s->head->next;
    heap::kfree(temp);

    return data;
}

bool stack::removeElement(Stack *s, void *data) {
    StackElement_t *temp, *prev;

    temp = s->head;
    if (temp == NULL) {
        return false;
    }

    if (temp->data == data) {
        pop(s);
        return true;
    }

    while (temp != NULL) {
        if (temp->data == data) {
            prev->next = temp->next;
            heap::kfree(temp);
            return true;
        }
        prev = temp;
        temp = temp->next;
    }

    return false;
}

void stack::print(Stack *s) {
    StackElement_t *temp;

    temp = s->head;
    while (temp != NULL) {
        stdio::kprintf("%d = 0x%x\n", temp->data, temp->data);
        temp = temp->next;
    }
}