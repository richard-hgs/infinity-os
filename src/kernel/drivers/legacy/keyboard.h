#pragma once
#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

// cpu
#include "isr.h"

/**
 * @brief PS/2 Keyboard errors should be greather than last ps2.h error since ps2 errors can be returned through keyboard errors.
 * 
 */
#define KBD_ERROR_GET_SCANCODE_ACK_ERROR 19
#define KBD_ERROR_GET_SCANCODE_ERROR 20

/**
 * @brief PS/2 Keyboard scancodes set
 * 
 */
#define KBD_SCAN_CODE_SET1 0x43 // Scancode set 1.
#define KBD_SCAN_CODE_SET2 0x41 // Scancode set 2 is the default scancode set. The 8042 translates scancode set 2 to scancode set 1.
#define KBD_SCAN_CODE_SET3 0x3F // Scancode set 3 is hard to keep compatibility.

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
 * | COMMAND |        DATA BYTES                         |             MEANING              |                                     RESPONSE                                     |
 * |         | LED states:                               |                                  |                                                                                  |
 * |         |  ____________________                     |                                  |                                                                                  |
 * |         | | BIT |     Use      |                    |                                  |                                                                                  |
 * |  0xED   | |  0  |  ScrollLock  |                    | Set Leds                         | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         | |  1  |  NumberLock  |                    |                                  |                                                                                  |
 * |         | |  2  |   CapsLock   |                    |                                  |                                                                                  |
 * |         |  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾                     |                                  |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                           | Echo (for diagnostic purposes,   |                                                                                  | 
 * |  0xEE   |                 None                      | and useful for device removal    | 0xEE (Echo) or 0xFE (Resend)                                                     | 
 * |         |                                           | detection)                       |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|------------------------------                                                    |
 * |  0xF0   | Sub-command:                              |                                  | 0xFA (ACK) or 0xFE (Resend) if scan code is being set; 0xFA (ACK) then the scan  |
 * |         |  ___________________________________      |                                  | code set number, or 0xFE (Resend) if you're getting the scancode. If getting the |
 * |         | | Value |           Use             |     |                                  | scancode the table indicates the value that identify each set:                   |
 * |         | |   0   | Get current scan code set |     |                                  |  _________________________                                                       |
 * |         | |   1   | Set scan code set 1       |     | Get/set current scan code set    | | Value |      Use        |                                                      |
 * |         | |   2   | Set scan code set 2       |     |                                  | | 0x43  | Scan code set 1 |                                                      |
 * |         | |   3   | Set scan code set 3       |     |                                  | | 0x41  | Scan code set 2 |                                                      |
 * |         |  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾      |                                  | | 0x3f  | Scan code set 3 |                                                      |
 * |         |                                           |                                  |  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾                                                       |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |  0xF2   |                 None                      |        Identify keyboard         | 0xFA (ACK) followed by none or more ID bytes (see "Detecting Device Types")      |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |  0xF3   | Typematic byte:                           |                                  |                                                                                  |
 * |         |  _______________________________________  |                                  |                                                                                  |
 * |         | | Bit/s |         Meaning               | |                                  |                                                                                  |
 * |         | |  4-0  | Repeat rate (0h=30Hz, 1F=2Hz) | |                                  |                                                                                  |
 * |         | |  6-5  | Delay before keys repeat      | | Set typematic rate and delay     | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         | |       | (0h=250ms, 1h=500ms, 2h=750,  | |                                  |                                                                                  |
 * |         | |       |  3h=1000ms)                   | |                                  |                                                                                  |
 * |         | |   7   | Must be zero                  | |                                  |                                                                                  |
 * |         |  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  |                                  |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |  0xF4   |                 None                      | Enable scanning                  | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         |                                           | Keyboard will send scan codes    |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                           | Disable scanning                 |                                                                                  |
 * |  0xF5   |                 None                      | Keyboard wont send scan codes    | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         |                                           | May also restore default params  |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |  0xF6   |                 None                      | Set default parameters           | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                           | Set all keys to typematic/auto-  |                                                                                  |
 * |  0xF7   |                 None                      | repeat only                      | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         |                                           | (scancode set 3 only)            |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                           | Set all keys to make/release     |                                                                                  |
 * |  0xF8   |                 None                      |                                  | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         |                                           | (scancode set 3 only)            |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |  0xF9   |                 None                      | Set all keys to make only        | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                           | Set all keys to typematic /      |                                                                                  |
 * |  0xFA   |                 None                      | autorepeat / make / release      | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         |                                           | (scancodeset 3 only)             |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                           | Set specific key to typematic /  |                                                                                  |
 * |  0xFB   |           Scancode for key                | autorepeat only                  | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         |                                           | (scancode set 3 only)            |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                           | Set specific key to make /       |                                                                                  |
 * |  0xFC   |           Scancode for key                | release                          | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         |                                           | (scancode set 3 only)            |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |         |                                           | Set specific key to make only    |                                                                                  |
 * |  0xFD   |           Scancode for key                |                                  | 0xFA (ACK) or 0xFE (Resend)                                                      |
 * |         |                                           | (scancode set 3 only)            |                                                                                  |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |  0xFE   |                 None                      | Resend last byte                 | Previously sent byte or 0xFE (Resend)                                            |
 * |---------|-------------------------------------------|----------------------------------|----------------------------------------------------------------------------------|
 * |  0xFF   |                 None                      | Reset and start self-test        | 0xAA (self-test passed), 0xFC or 0xFD (self test failed), or 0xFE (Resend)       |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 * Special Bytes:
 *  ___________________________________________________________________________________________
 * | Response Byte |          Meaning                                                          |
 * |     0x00      | Key detection error or internal buffer overrun                            |
 * |     0xAA      | Self test passed (sent after "0xFF (reset)" command or keyboard power up) |
 * |     0xEE      | Response to "0xEE (echo)" command                                         |
 * |     0xFA      | Command acknowledged (ACK)                                                |
 * | 0xFC and 0xFD | Self test failed (sent after "0xFF (reset)" command or keyboard power up) |
 * |     0xFE      | Resend (keyboard wants controller to repeat last command it sent)         |
 * |     0xFF      | Key detection error or internal buffer overrun                            |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 * All other bytes sent by the keyboard are scan codes, where interpretation depends on the currently selected scan code set.
 * 
 * SCAN_CODE_SETS:
 *     - Keyboard keys can be define of 3 types:
 *         - (keypad)      A key that is on the numerics keypad (typically found on the right hand side of the keyboard).
 *         - (ACPI)        A key that is part of the "ACPI" group of keys (typically found near the top of the keyboard). Can shutdown pc, sleep, etc.
 *         - (multimedia)  A key that is part of the multimedia group of keys (typically found near the top of the keyboard). Play song, next song, etc.
 * 
 * ASCII_TABLE:
 *  ___________________________________________________________________________________________________________________________________
 * | DEC | HEX  | VALUE |     | DEC | HEX  | VALUE |      | DEC | HEX  | VALUE |      | DEC | HEX  | VALUE |      | DEC | HEX  | VALUE |
 * |   0 | 00h  |  NUL  |     |  26 | 1Ah  |  SUB  |      |  52 | 34h  |   4   |      |  78 | 4Eh  |   N   |      | 104 | 68h  |   h   |
 * |   1 | 01h  |  SOH  |     |  27 | 1Bh  |  ESC  |      |  53 | 35h  |   5   |      |  79 | 4Fh  |   O   |      | 105 | 69h  |   i   |
 * |   2 | 02h  |  STX  |     |  28 | 1Ch  |  FS   |      |  54 | 36h  |   6   |      |  80 | 50h  |   P   |      | 106 | 6Ah  |   j   |
 * |   3 | 03h  |  ETX  |     |  29 | 1Dh  |  GS   |      |  55 | 37h  |   7   |      |  81 | 51h  |   Q   |      | 107 | 6Bh  |   k   |
 * |   4 | 04h  |  EOT  |     |  30 | 1Eh  |  RS   |      |  56 | 38h  |   8   |      |  82 | 52h  |   R   |      | 108 | 6Ch  |   l   |
 * |   5 | 05h  |  ENQ  |     |  31 | 1Fh  |  US   |      |  57 | 39h  |   9   |      |  83 | 53h  |   S   |      | 109 | 6Dh  |   m   |
 * |   6 | 06h  |  ACK  |     |  32 | 20h  | SPACE |      |  58 | 3Ah  |   :   |      |  84 | 54h  |   T   |      | 110 | 6Eh  |   n   |
 * |   7 | 07h  |  BEL  |     |  33 | 21h  |   !   |      |  59 | 3Bh  |   ;   |      |  85 | 55h  |   U   |      | 111 | 6Fh  |   o   |
 * |   8 | 08h  |  BS   |     |  34 | 22h  |   "   |      |  60 | 3Ch  |   <   |      |  86 | 56h  |   V   |      | 112 | 70h  |   p   |
 * |   9 | 09h  |  HT   |     |  35 | 23h  |   #   |      |  61 | 3Dh  |   =   |      |  87 | 57h  |   W   |      | 113 | 71h  |   q   |
 * |  10 | 0Ah  |  LF   |     |  36 | 24h  |   $   |      |  62 | 3Eh  |   >   |      |  88 | 58h  |   X   |      | 114 | 72h  |   r   |
 * |  11 | 0Bh  |  VT   |     |  37 | 25h  |   %   |      |  63 | 3Fh  |   ?   |      |  89 | 59h  |   Y   |      | 115 | 73h  |   s   |
 * |  12 | 0Ch  |  FF   |     |  38 | 26h  |   &   |      |  64 | 40h  |   @   |      |  90 | 5Ah  |   Z   |      | 116 | 74h  |   t   |
 * |  13 | 0Dh  |  CR   |     |  39 | 27h  |   '   |      |  65 | 41h  |   A   |      |  91 | 5Bh  |   [   |      | 117 | 75h  |   u   |
 * |  14 | 0Eh  |  SO   |     |  40 | 28h  |   (   |      |  66 | 42h  |   B   |      |  92 | 5Ch  |   \   |      | 118 | 76h  |   v   |
 * |  15 | 0Fh  |  SI   |     |  41 | 29h  |   )   |      |  67 | 43h  |   C   |      |  93 | 5Dh  |   ]   |      | 119 | 77h  |   w   |
 * |  16 | 10h  |  DLE  |     |  42 | 2Ah  |   *   |      |  68 | 44h  |   D   |      |  94 | 5Eh  |   ^   |      | 120 | 78h  |   x   |
 * |  17 | 11h  |  DC1  |     |  43 | 2Bh  |   +   |      |  69 | 45h  |   E   |      |  95 | 5Fh  |   _   |      | 121 | 79h  |   y   |
 * |  18 | 12h  |  DC2  |     |  44 | 2Ch  |   ,   |      |  70 | 46h  |   F   |      |  96 | 60h  |   `   |      | 122 | 7Ah  |   z   |
 * |  19 | 13h  |  DC3  |     |  45 | 2Dh  |   -   |      |  71 | 47h  |   G   |      |  97 | 61h  |   a   |      | 123 | 7Bh  |   {   |
 * |  20 | 14h  |  DC4  |     |  46 | 2Eh  |   .   |      |  72 | 48h  |   H   |      |  98 | 62h  |   b   |      | 124 | 7Ch  |   |   |
 * |  21 | 15h  |  NAK  |     |  47 | 2Fh  |   /   |      |  73 | 49h  |   I   |      |  99 | 63h  |   c   |      | 125 | 7Dh  |   }   |
 * |  22 | 16h  |  SYN  |     |  48 | 30h  |   0   |      |  74 | 4Ah  |   J   |      | 100 | 64h  |   d   |      | 126 | 7Eh  |   ~   |
 * |  23 | 17h  |  ETB  |     |  49 | 31h  |   1   |      |  75 | 4Bh  |   K   |      | 101 | 65h  |   e   |      | 127 | 7Fh  |  DEL  |
 * |  24 | 18h  |  CAN  |     |  50 | 32h  |   2   |      |  76 | 4Ch  |   L   |      | 102 | 66h  |   f   |      |     |      |       |
 * |  25 | 19h  |  EM   |     |  51 | 33h  |   3   |      |  77 | 4Dh  |   M   |      | 103 | 67h  |   g   |      |     |      |       |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 */
namespace kbd {
    /**
     * @brief Install function is called after device initialization success by the ps2.h install(); function.
     *        This function will not be called if the keyboard is not present or an error happens inside ps2 controller install function.
     */
    uint8_t install();

    /**
     * @brief Keyboard interruption handler, each time a key is pressed this interruption will be dispatched.
     *        Then we need to read the data from ps2 controller data port
     * 
     * @param r Pushed register of the given interruption
     */
    void keyboardIntHandler(registers_t* r);

    /**
     * @brief Get the Current Scan Code of the keyboard device. This communicates with keyboard. 
     * 
     * @param scs        OUT - Scan code set
     * @return uint8_t   0=PS2_NO_ERROR or the ps2.h error or keyboard.h error.
     */
    uint8_t getCurrentScanCodeSet(uint8_t* scs);
}

#endif