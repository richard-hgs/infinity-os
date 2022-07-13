#pragma once
#ifndef _PIT_H_
#define _PIT_H_

namespace pit {
    /**
     * @brief PIT - Install the Programmable Interval Timer and perform initial configuration
     *        This register a isr handler in isr.cpp interruptHandlers to receive the timer interruption
     * 
     */
    void install();

    /**
     * @brief PIT - Enable the pit interruptions to be sent to the cpu.
     *              This clear the mask in PIC IRQ0 Line
     * 
     */
    void enable();

    /**
     * @brief PIT - Disable the pit interruptions to be sent to the cpu.
     *              This set the mask in PIC IRQ0 Line
     * 
     */
    void disable();
} 

#endif