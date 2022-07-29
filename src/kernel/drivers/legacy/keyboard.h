#pragma once
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

// cpu
#include "isr.h"

/**
 * @brief The PS/2 Keyboard 
 * 
 * - Is a device that talks to a PS/2 controller using serial communication. 
 * - The PS/2 Keyboard accepts commands and sends responses to those commands, and also sends scan codes indicating when a key was pressed or released.
 * 
 * COMMANDS: 
 *     - A command is One Byte
 *     - Some commands have data byte/s which must be sent after the command byte.
 *     - The keyboard typically responds to a command by sending either an "ACK" (to acknowledge the command). 
 *       or a "Resend" (to say something was wrong with the previous command)
 *  _______________________________________________________________________________________________________________________________________________________________________
 * | COMMAND |        DATA BYTES                     |             MEANING              |                                     RESPONSE                                     |
 * |         | LED states:                           |                                  |                                                                                  |
 * |         |  ____________________                 |                                  |                                                                                  |
 * |         | | BIT |     Use      |                |                                  |                                                                                  |
 * |  0xED   | |  0  |  ScrollLock  |                | Set Leds                         | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         | |  1  |  NumberLock  |                |                                  |                                                                                  |
 * |         | |  2  |   CapsLock   |                |                                  |                                                                                  |
 * |         |  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾                 |                                  |                                                                                  |
 * |---------|---------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                       | Echo (for diagnostic purposes,   |                                                                                  | 
 * |  0xEE   |                 None                  | and useful for device removal    | 0xEE (Echo) or 0xFE (Resend)                                                     | 
 * |         |                                       | detection)                       |                                                                                  |
 * |---------|---------------------------------------|----------------------------------|------------------------------                                                    |
 * |  0xF0   | Sub-command:                          |                                  | 0xFA (ACK) or 0xFE (Resend) if scan code is being set; 0xFA (ACK) then the scan  |
 * |         |  ___________________________________  |                                  | code set number, or 0xFE (Resend) if you're getting the scancode. If getting the |
 * |         | | Value |           Use             | |                                  | scancode the table indicates the value that identify each set:                   |
 * |         | |   0   | Get current scan code set | |                                  |  _________________________                                                       |
 * |         | |   1   | Set scan code set 1       | | Get/set current scan code set    | | Value |      Use        |                                                      |
 * |         | |   2   | Set scan code set 2       | |                                  | | 0x43  | Scan code set 1 |                                                      |
 * |         | |   3   | Set scan code set 3       | |                                  | | 0x41  | Scan code set 2 |                                                      |
 * |         |  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  |                                  | | 0x3f  | Scan code set 3 |                                                      |
 * |         |                                       |                                  |  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾                                                       |
 * |---------|---------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |  0xF2   |                 None                  |        Identify keyboard         | 0xFA (ACK) followed by none or more ID bytes (see "Detecting Device Types")      |
 * |---------|---------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                       |                                  |                                                                                  |
 * |         |                                       |                                  |                                                                                  |
 * |         |                                       |                                  |                                                                                  |
 * |         |                                       |                                  |                                                                                  |
 */
namespace kbd {
    /**
     * @brief Keyboard interruption handler, each time a key is pressed this interruption will be dispatched.
     *        Then we need to read the data from ps2 controller data port
     * 
     * @param r Pushed register of the given interruption
     */
    void keyboardIntHandler(registers_t* r);
}

#endif