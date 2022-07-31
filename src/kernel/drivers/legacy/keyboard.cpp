// libc
#include <stdint.h>
// stdlibs
#include "stdlib.h"
// drivers
#include "ps2.h"
// stdlibs
#include "stdio.h"
#include "keyboard.h"

#define CMD_GET_SET_SCANCODE_SET 0xF0    // Get/set current scan code set
#define CMD_DATA_GET_SCANCODE_SET 0x0    // Get current scan code set

uint8_t lastKey = 0;        // Last key pressed

bool _capslock;
bool _shift;
bool _ctrl;

const char* _qwertyuiop = "qwertyuiop";
const char* _asdfghjkl = "asdfghjkl";
const char* _zxcvbnm = "zxcvbnm";

typedef enum SCS1_en {
    KEY_NULL = 0x00,

    KEY_ESC      = 0x01, KEY_F1         = 0x3B, KEY_F2   = 0x3C, KEY_F3    = 0x3D, KEY_F4 = 0x3E, KEY_F5 = 0x3F, KEY_F6 = 0x3F, KEY_F7 = 0x41, KEY_F8 = 0x42, KEY_F9    = 0x43, KEY_F10       = 0x44, KEY_F11       = 0x57, KEY_F12       = 0x58, 
    KEY_1        = 0x02, KEY_2          = 0x03, KEY_3    = 0x04, KEY_4     = 0x05, KEY_5  = 0x06, KEY_6  = 0x07, KEY_7  = 0x08, KEY_8  = 0x09, KEY_9  = 0x0A, KEY_0     = 0x0B, KEY_MINUS     = 0x0C, KEY_EQUAL     = 0x0D, KEY_BACKSPACE = 0x0E,
    KEY_TAB      = 0x0F, KEY_Q          = 0x10, KEY_W    = 0x11, KEY_E     = 0x12, KEY_R  = 0x13, KEY_T  = 0x14, KEY_Y  = 0x15, KEY_U  = 0x16, KEY_I  = 0x17, KEY_O     = 0x18, KEY_P         = 0x19, KEY_O_BRACKET = 0x1A, KEY_C_BRACKET = 0x1B, KEY_ENTER = 0x1C,
    KEY_CAPSLOCK = 0x3A, KEY_A          = 0x1E, KEY_S    = 0x1F, KEY_D     = 0x20, KEY_F  = 0x21, KEY_G  = 0x22, KEY_H  = 0x23, KEY_J  = 0x24, KEY_K  = 0x25, KEY_L     = 0x26, KEY_SEMICOLON = 0x27, KEY_S_QUOTE   = 0x28, KEY_GRAVE     = 0x29,
    KEY_LSHIFT   = 0x2A, KEY_BACKSLASH  = 0x2B, KEY_Z    = 0x2C, KEY_X     = 0x2D, KEY_C  = 0x2E, KEY_V  = 0x2F, KEY_B  = 0x30, KEY_N  = 0x31, KEY_M  = 0x32, KEY_COMMA = 0x33, KEY_PERIOD    = 0x34, KEY_SLASH     = 0x35, KEY_RSHIFT    = 0x36, 
    KEY_LCTRL    = 0x1D, /*LGUI = 0xE0, 0x5B*/  KEY_LALT = 0x38, KEY_SPACE = 0x39, /*KEY_RALT=0xE0, 0x38*/
} SCS1_en;

uint8_t kbd::install() {
    uint8_t tmp;
    stdio::kprintf("KBD - install\n");

    return PS2_NO_ERROR;
}

void kbd::keyboardIntHandler(registers_t* r) {
    uint8_t curKey;
    unsigned char asciiKey = 0;
    ps2::readData(&curKey); // Read the data returned by the keyboard in PS/2 Controller data

    if (curKey >= 0x81 && curKey <= 0xD8) { // Key released
        curKey -= 0x80;                     // Transform the released key code in a pressed key code
        if (curKey == KEY_LSHIFT || curKey == KEY_RSHIFT) {
            _shift = false;
        }
    } else if (curKey >= KEY_1 && curKey <= KEY_9) { // Is numeric
        asciiKey = curKey + 47;           // Offset in ascii table to the first numeric 1 char. Since numbers are in sequence resolve them
    } else if (curKey == KEY_0) {             // Since number isn't the last number in ascii we need to write its value manually
        asciiKey = '0';
    } else if (curKey >= KEY_Q && curKey <= KEY_P) { // Since codes are in sequence subtract them to point to same chars of the buffer.
        asciiKey = _qwertyuiop[curKey - KEY_Q];
    } else if (curKey >= KEY_A && curKey <= KEY_L) { // Since codes are in sequence subtract them to point to same chars of the buffer.
        asciiKey = _asdfghjkl[curKey - KEY_A];
    } else if (curKey >= KEY_Z && curKey <= KEY_M) { // Since codes are in sequence subtract them to point to same chars of the buffer.
        asciiKey = _zxcvbnm[curKey - KEY_Z];
    } else if (curKey == KEY_TAB) {                  // Horizontal tab
        asciiKey = '\t';
    } else if (curKey == KEY_BACKSPACE) {
        asciiKey = '\b';
    } else if (curKey == KEY_SPACE) {
        asciiKey = ' ';
    } else if (curKey == KEY_LSHIFT || curKey == KEY_RSHIFT) {
        _shift = true;
    } else if (curKey == KEY_CAPSLOCK) {
        _capslock = !_capslock;
    }

    if ((!_capslock && _shift || _capslock && !_shift) && asciiKey >= 0x61 && asciiKey <= 0x7A) { // Shift or Capslock pressed change from ASCII 0x61=a ... 0x7A=z to ASCII 0x41=A ... 0x5A=Z
        asciiKey -= 0x20; // subtract from ASCII a to ASCII A equivalent.
    }

    stdio::kprintf("%c", asciiKey);

    lastKey = curKey;

    // stdio::kprintf("KBD - lastKey %02x - curKey: %02x - lastKeyAscii: %c\n", lastKey, curKey, (lastKeyAscii != 0 ? lastKeyAscii : ' '));
}

uint8_t kbd::getCurrentScanCodeSet(uint8_t* scanCodeSet) {
    uint8_t result;
    BufferContains_t bufferContains[3];
    BufferContains_t bufferContains2[3];

    // Command to get keyboard scan code
    bufferContains[0] = {DEVICE_RESP_ACK, KBD_ERROR_GET_SCANCODE_ACK_ERROR};
    result = ps2::sendDataToDevice(PS2_DEVICE_TYPE_KEYBOARD, CMD_GET_SET_SCANCODE_SET, bufferContains, 1, NULL, 0, NULL, NULL, NULL);
    if (result != PS2_NO_ERROR) {
        return result;
    }
    
    // Sub command to get keyboard scan code
    bufferContains2[0] = {KBD_SCAN_CODE_SET1, KBD_ERROR_GET_SCANCODE_ERROR}; // Wait until response contains one of scan code sets
    bufferContains2[1] = {KBD_SCAN_CODE_SET2, KBD_ERROR_GET_SCANCODE_ERROR}; // If one found return inside scanCodeSet
    bufferContains2[2] = {KBD_SCAN_CODE_SET3, KBD_ERROR_GET_SCANCODE_ERROR};
    result = ps2::sendDataToDevice(PS2_DEVICE_TYPE_KEYBOARD, CMD_DATA_GET_SCANCODE_SET, bufferContains, 1, bufferContains2, 3, scanCodeSet, NULL, NULL);
    
    return result;
}