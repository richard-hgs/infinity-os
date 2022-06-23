#include "vga.h"
#include "io.h"
#include "memutils.h"
#include "stdlib.h"     // debug only
#include <stdint.h>

#define WIDTH 80
#define HEIGHT 25

#define VGA_ADDRESS 0xB8000
#define VGA_BUFFER ((uint16_t*) VGA_ADDRESS)

// The Screen max offset pos
#define SCREEN_MAX_OFFSET_POS (VGA_ADDRESS + (WIDTH * HEIGHT * 2))
// Make a vga color attribute byte
#define MAKE_COLOR(bg, fg) ((bg << 4) | fg)
// Make a vga text mode char (2 bytes), bg = backgroundColor, fg = ForegroundColor
#define PAINT(c, bg, fg) (((MAKE_COLOR(bg, fg)) << 8) | (c & 0xFF))
// Get current vga cursor offset position
#define VGA_OFFSET_CURSOR_POS(currentVgaAddr) (static_cast<uint16_t>((((int) currentVgaAddr) - VGA_ADDRESS)) / 2)
// Get the Row position from vga cursor offset position
#define ROW_FROM_OFFSET_CURSOR_POS(offset) (offset / WIDTH)
// Get the Col position from vga cursor offset position
#define COL_FROM_OFFSET_CURSOR_POS(offset) (offset % WIDTH)
// Get the Screen Offset Pos given a col and row value
#define GET_SCREEN_OFFSET_POS(col, row) (row * WIDTH + col)

// ===================== PRIVATE =======================

/**
 * @brief Get current vga cursor offset position
 *
 * @return uint16_t Offset position
 */
uint16_t getCursorOffsetPos() {
    uint16_t offset = 0;
    io::outb(0x3D4, 0x0F);
    offset = offset | io::inb(0x3D5);
    io::outb(0x3D4, 0x0E);
    offset = offset | ((uint16_t)io::inb(0x3D5)) << 8;
    return offset;
}

/**
 * @brief Set current vga cursor offset position
 * 
 * @param offsetPos 
 */
void setCursorOffsetPos(uint16_t offsetPos) {
    io::outb(0x3D4, 14);
    io::outb(0x3D5, offsetPos >> 8);
    io::outb(0x3D4, 15);
    io::outb(0x3D5, offsetPos);
}

// ====================== PUBLIC =======================

void vga::printStr(const char* str) {
    vga::printStr(VGA_DEF_FORECOLOR, VGA_DEF_BGCOLOR, str);
}

void vga::printStr(int foreColor, int bgColor, const char* str) {
    // Get current vga cursor offset position
    uint16_t cursorOffset = getCursorOffsetPos();

    // Video memory base offset
    volatile uint16_t* video = (volatile uint16_t*) VGA_ADDRESS + cursorOffset;

    // Offset video memory to start at current cursor position
    //  - Since it is a short 16 bits variable should be sum in increments of 2 bytes positions
    while (*str != 0) {
        if (*str == '\n') {
            // Line break - Discard \n char
            str++;
            // Increment video buffer to go to next line
            // E.g.:
            // WIDTH = 80;
            // VGA_OFFSET_CURSOR_POS = 30;
            // OFFSET_INCREMENT = 30 = VGA_OFFSET_CURSOR_POS % 80;
            // VIDEO_BUFFER_INCREMENT = 50 = WIDTH - OFFSET_INCREMENT;
            //  - Increment video address by 50 to advance to the next line
            // E.g. 2:
            // WIDTH = 80;
            // VGA_OFFSET_CURSOR_POS = 90;
            // OFFSET_INCREMENT = 10 = VGA_OFFSET_CURSOR_POS % 80;
            // VIDEO_BUFFER_INCREMENT = 70 = WIDTH - OFFSET_INCREMENT;
            //  - Increment video address by 70 to advance to the next line
            video += WIDTH - (VGA_OFFSET_CURSOR_POS((int) video) % WIDTH);
        } else {
            *video++ = PAINT(*str++, bgColor, foreColor);
        }

        // Check if video max address reached and scroll content up
        if (((int) video) >= SCREEN_MAX_OFFSET_POS) {
            void* startAddress = (void*) (VGA_BUFFER + WIDTH);           // startAddress = The start ptr address of the second vga text buffer line
            uint32_t sizeToCopy = (HEIGHT - 1) * WIDTH * 2;              // sizeToCopy = Size of the buffer of all lines from second line until last line
            memutils::memcpy(VGA_BUFFER, startAddress, sizeToCopy);      // copy the lines up by one line

            // While current video ptr address is greather than start ptr address of the last line
            // decrement blanking all chars of the last line and position cursor at the end char of the
            // penultimate line, so we need to increment video ptr address to move cursor to the last line.
            // this operation is needed because the first char of the last line was not blanked
            while(((int) video) >= (VGA_ADDRESS + (WIDTH * (HEIGHT - 1) * 2))) {
                *video-- = PAINT(0x20, bgColor, foreColor);
            }
            video++;
        }
    }

    // // USED FOR DEBUG
    // char strCurOffset[33] = {0};
    // stdlib::itoa(SCREEN_MAX_OFFSET_POS, 10, strCurOffset);
    // const char* mStrCurOffset = strCurOffset;

    // while (*mStrCurOffset != 0) {
    //     if (*mStrCurOffset == '\n') {
    //         // Line break
    //         *mStrCurOffset++;
    //         //video += 1;
    //     } else {
    //         *video++ = PAINT(*mStrCurOffset++, bgColor, foreColor);
    //     }
    // }

    // Update cursor offset position
    setCursorOffsetPos(VGA_OFFSET_CURSOR_POS((int) video));
}

void vga::clearScreen() {
    vga::clearScreen(VGA_DEF_FORECOLOR, VGA_DEF_BGCOLOR);
}

void vga::clearScreen(int foreColor, int bgColor) {
    // Write 0x20 = ' ' blank char in entire vga buffer
    vga::setCursorPosition(0, 0);
    memutils::memset_16(VGA_BUFFER, PAINT(0x20, bgColor, foreColor), WIDTH * HEIGHT);
}

void vga::getCursorPosition(int* row, int* col) {
    uint16_t offset = getCursorOffsetPos();
    *row = ROW_FROM_OFFSET_CURSOR_POS(offset);
    *col = COL_FROM_OFFSET_CURSOR_POS(offset);
}

void vga::setCursorPosition(int col, int row) {
    unsigned offset_pos = GET_SCREEN_OFFSET_POS(col, row);
    setCursorOffsetPos(offset_pos);
}