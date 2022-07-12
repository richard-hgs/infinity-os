#pragma once
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

namespace keyboard {
    /**
     * @brief Initialize and configure the PS/2 Keyboard with PIC IRQs located at isr.cpp
     * 
     */
    void install();
}

#endif