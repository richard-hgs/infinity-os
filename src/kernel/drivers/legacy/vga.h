#pragma once
#ifndef _VGA_H_
#define _VGA_H_

#include <stdbool.h>

/* VGA (FOREGROUND | BACKGROUND) COLORS */
#define VGA_BLACK           0
#define VGA_BLUE            1
#define VGA_GREEN           2
#define VGA_CYAN            3
#define VGA_RED             4
#define VGA_MAGENTA         5
#define VGA_BROWN           6
#define VGA_LIGHT_GREY      7
#define VGA_DARK_GREY       8
#define VGA_LIGHT_BLUE      9
#define VGA_LIGHT_GREEN     10
#define VGA_LIGHT_CYAN      11
#define VGA_LIGHT_RED       12
#define VGA_LIGHT_MAGENTA   13
#define VGA_LIGHT_BROWN     14
#define VGA_WHITE           15

/** VGA DEFAULT (FOREGROUND | BACKGROUND) COLORS */
#define VGA_DEF_FORECOLOR VGA_WHITE
#define VGA_DEF_BGCOLOR VGA_BLACK

/**
 * @brief LEGACY VGA (Text Mode) 80x86 
 * 
 * usually located at RAM address: 0xB8000
 * Writing bytes in the below template to print to vga screen
 *  ________________________________________________________________________
 * |             Attribute				|		      Character				|
 * |------------------------------------|-----------------------------------|			
 * |     7     | 6	| 5 | 4 | 3 | 2 | 1 | 0 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * |-----------|------------|-----------|-----------------------------------|
 * |Blink[n 1] |  Bg color  |Foreg color|    [a-z][A-Z][0-9][!@#$%&*()]     |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 * - 1.7 Depending on the mode setup, attribute bit 7 may be either the blink bit or the fourth background color bit 
 *       (which allows all 16 colors to be used as background colours).
 * 
 * - 1.3 Attribute bit 3 (foreground intensity) also selects between fonts A and B (see below). Therefore, if these fonts 
 *       are not the same, this bit is simultaneously an additional code point bit.
 * 
 * - 1.0 Attribute bit 0 also enables underline, if certain other attribute bits are set to zero.
 * 
 * - UNDERLINE:
 *   - MDA-compatible feature. The attribute bits not used by the MDA must be set to zero or the underline will not be shown.
 *     text in only two colors can be underlined (5555FF and 0000AA with the default palette).
 *   - Switching to an MDA-compatible monochrome text mode makes the underline appear on screen.
 * 
 * - FONTS:
 *   - Monospace raster fonts containing 256 glyphs.
 *   - All glyphs in a font are the same size, but this size can be changed. Typically, 
 *     glyphs are 8 dots wide and 8–16 dots high, however the height can be any value up to a maximum of 32.
 * 
 * - CURSOR:
 *   - The shape of the cursor is restricted to a rectangle the full width of the character box, and filled with the 
 *     foreground color of the character at the cursor's current location.
 *   - A mouse cursor in TUI (when implemented) is not usually the same thing as a hardware cursor, but a moving rectangle 
 *     with altered background or a special glyph.
 * 
 * - ESCAPESEQ:
 *   - \b - Backspace;
 *   - \t - Tabulation;
 * 
 * - MORE:
 *   - To know more about vga access folder docs and search for soft_vga.pdf file
 */
namespace vga {
    /**
     * @brief Print text in vga using default foreColor and bgColor
     * 
     * @param str Text to be printed
     */
    void printStr(const char *str);

    /**
     * @brief Print text in vga
     * 
     * @param foreColor Text color E.g VGA_WHITE
     * @param bgColor Text background color E.g VGA_BLACK
     * @param str Text to be printed
     */
    void printStr(int foreColor, int bgColor, const char *str);

    /**
     * @brief Clear the vga text using default foreColor and bgColor and set cursor
     * 
     */
    void clearScreen();

    /**
     * @brief Clear the vga text
     * 
     * @param foreColor Text color E.g VGA_WHITE
     * @param bgColor Text background color E.g VGA_BLACK
     * @param setCursor true=Reset cursor position, false=Cursor where it was before.
     */
    void clearScreen(int foreColor, int bgColor, bool setCursor);

    /**
     * @brief Get the vga Cursor position
     * 
     * @param row 
     * @param col 
     */
    void getCursorPosition(int *row, int *col);

    /**
     * @brief Set the vga Cursor position
     * 
     * @param row 
     * @param col 
     */
    void setCursorPosition(int col, int row);

    /**
     * @brief Set the Vga Address location.
     * This method is used when the vga address is mapped to another location in memory
     * Usually set by the paging.cpp mechanism
     * 
     * @param vgaAddress 
     */
    void setVgaAddress(int newVgaAddress);
}
#endif