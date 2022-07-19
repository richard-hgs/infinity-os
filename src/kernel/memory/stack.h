#pragma once
#ifndef _STACK_H_
#define _STACK_H_

#include <stdbool.h>

typedef struct StackElement {
    void *data;
    struct StackElement *next;
} StackElement_t;

typedef struct {
    StackElement_t *head;
} Stack;

namespace stack {
    /**
     * @brief Initialize the given stack.
     * 
     * @param s Stack being initialized.
     */
    void init(Stack *s);

    /**
     * @brief Push a data pointer reference onto the stack instance.
     * 
     * @param s     Stack that will get some data pushed.
     * @param data  Data being pushed onto the stack.
     * @return int  0=FAILURE, 1=SUCCESS
     */
    int push(Stack *s, void *data);

    /**
     * @brief Pop a data pointer reference from the given stack instance.
     * 
     * @param s         Stack instance.
     * @return void*    Pointer being pop.
     */
    void *pop(Stack *s);

    /**
     * @brief Remove a element from the ginve stack instance.
     * 
     * @param s         Stack instance.
     * @param data      Data pointer reference being removed.
     * @return true     Element removed
     * @return false    Element not removed, maybe not found
     */
    bool removeElement(Stack *s, void *data);

    /**
     * @brief Print on the screen the entire data pointer refs that is in the stack in (%d=decimal, %x=hexadecimal format) one by one.
     * 
     * @param s     Stack instance being printed.
     */
    void print(Stack *s);
}

#endif