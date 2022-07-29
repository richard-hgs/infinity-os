// drivers
#include "ps2.h"
// stdlibs
#include "stdio.h"

#include "keyboard.h"

// Append keys one after another comma sepparated inside an array
// #define APPEND_KEYS(K, ...) K, ##__VA_ARGS__

/**
 * @brief Enum with all Keyboard Key Codes Mapped to int
 * 
 */
typedef enum KEYCODE {
	/* Alphanumeric keys */
	KEY_SPACE	= ' ',
	KEY_0		= '0',
	KEY_1		= '1',
	KEY_2		= '2',
	KEY_3		= '3',
	KEY_4		= '4',
	KEY_5		= '5',
	KEY_6		= '6',
	KEY_7		= '7',
	KEY_8		= '8',
	KEY_9		= '9',

	KEY_A		= 'a',
	KEY_B		= 'b',
	KEY_C		= 'c',
	KEY_D		= 'd',
	KEY_E		= 'e',
	KEY_F		= 'f',
	KEY_G		= 'g',
	KEY_H		= 'h',
	KEY_I		= 'i',
	KEY_J		= 'j',
	KEY_K		= 'k',
	KEY_L		= 'l',
	KEY_M		= 'm',
	KEY_N		= 'n',
	KEY_O		= 'o',
	KEY_P		= 'p',
	KEY_Q		= 'q',
	KEY_R		= 'r',
	KEY_S		= 's',
	KEY_T		= 't',
	KEY_U		= 'u',
	KEY_V		= 'v',
	KEY_W		= 'w',
	KEY_X		= 'x',
	KEY_Y		= 'y',
	KEY_Z		= 'z',

	KEY_RETURN	= '\r',
	KEY_ESCAPE	= 0x1001,
	KEY_BACKSPACE	= '\b',

	/* Arrow keys */
	KEY_UP		= 0x1100,
	KEY_DOWN	= 0x1101,
	KEY_LEFT	= 0x1102,
	KEY_RIGHT	= 0x1103,

	/* Function keys */
	KEY_F1		= 0x1201,
	KEY_F2		= 0x1202,
	KEY_F3		= 0x1203,
	KEY_F4		= 0x1204,
	KEY_F5		= 0x1205,
	KEY_F6		= 0x1206,
	KEY_F7		= 0x1207,
	KEY_F8		= 0x1208,
	KEY_F9		= 0x1209,
	KEY_F10	= 0x120a,
	KEY_F11	= 0x120b,
	KEY_F12	= 0x120c,
	KEY_F13	= 0x120d,
	KEY_F14	= 0x120e,
	KEY_F15	= 0x120f,

	KEY_DOT               = '.',
	KEY_COMMA             = ',',
	KEY_COLON             = ':',
	KEY_SEMICOLON         = ';',
	KEY_SLASH             = '/',
	KEY_BACKSLASH         = '\\',
	KEY_PLUS              = '+',
	KEY_MINUS             = '-',
	KEY_ASTERISK          = '*',
	KEY_EXCLAMATION       = '!',
	KEY_QUESTION          = '?',
	KEY_QUOTEDOUBLE       = '\"',
	KEY_QUOTE             = '\'',
	KEY_EQUAL             = '=',
	KEY_HASH              = '#',
	KEY_PERCENT           = '%',
	KEY_AMPERSAND         = '&',
	KEY_UNDERSCORE        = '_',
	KEY_LEFTPARENTHESIS   = '(',
	KEY_RIGHTPARENTHESIS  = ')',
	KEY_LEFTBRACKET       = '[',
	KEY_RIGHTBRACKET      = ']',
	KEY_LEFTCURL          = '{',
	KEY_RIGHTCURL         = '}',
	KEY_DOLLAR            = '$',
	KEY_POUND             = '£',
	KEY_EURO              = '$',
	KEY_LESS              = '<',
	KEY_GREATER           = '>',
	KEY_BAR               = '|',
	KEY_GRAVE             = '`',
	KEY_TILDE             = '~',
	KEY_AT                = '@',
	KEY_CARRET            = '^',

	/* Numeric keypad */
	KEY_KP_0              = '0',
	KEY_KP_1              = '1',
	KEY_KP_2              = '2',
	KEY_KP_3              = '3',
	KEY_KP_4              = '4',
	KEY_KP_5              = '5',
	KEY_KP_6              = '6',
	KEY_KP_7              = '7',
	KEY_KP_8              = '8',
	KEY_KP_9              = '9',
	KEY_KP_PLUS           = '+',
	KEY_KP_MINUS          = '-',
	KEY_KP_DECIMAL        = '.',
	KEY_KP_DIVIDE         = '/',
	KEY_KP_ASTERISK       = '*',
	KEY_KP_NUMLOCK        = 0x300f,
	KEY_KP_ENTER          = 0x3010,

	KEY_TAB               = 0x4000,
	KEY_CAPSLOCK          = 0x4001,

	/* Modify keys */
	KEY_LSHIFT            = 0x4002,
	KEY_LCTRL             = 0x4003,
	KEY_LALT              = 0x4004,
	KEY_LWIN              = 0x4005,
	KEY_RSHIFT            = 0x4006,
	KEY_RCTRL             = 0x4007,
	KEY_RALT              = 0x4008,
	KEY_RWIN              = 0x4009,

	KEY_INSERT            = 0x400a,
	KEY_DELETE            = 0x400b,
	KEY_HOME              = 0x400c,
	KEY_END               = 0x400d,
	KEY_PAGEUP            = 0x400e,
	KEY_PAGEDOWN          = 0x400f,
	KEY_SCROLLLOCK        = 0x4010,
	KEY_PAUSE             = 0x4011,

	KEY_UNKNOWN,
	KEY_NUMKEYCODES
} KEYCODE;

// MAP in ordinal order
int _kbd_std_table[] = {
//      00         01        02      03    04     05
	KEY_UNKNOWN, KEY_ESCAPE, KEY_1, KEY_2, KEY_3, KEY_4,
//   06      07     08     09     0A     0B      0C
	KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS,
//    0D              0E         F        10    11     12
	KEY_EQUAL, KEY_BACKSPACE, KEY_TAB, KEY_Q, KEY_W, KEY_E,
//   13      14     15     16    17      18    19
    KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P,
//        1A               1B             1C          1D 
	KEY_LEFTBRACKET, KEY_RIGHTBRACKET, KEY_RETURN, KEY_LCTRL,
//    1E     1F     20    21     22     23     24      25     26
	KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L,
//    27               28          29        2A           2B
	KEY_SEMICOLON, KEY_QUOTE, KEY_GRAVE, KEY_LSHIFT, KEY_BACKSLASH,
//   2C    2D     2E      2F    30     31      32      33
	KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA,
//    34         35         36            37             38
	KEY_DOT, KEY_SLASH, KEY_RSHIFT, KEY_KP_ASTERISK, KEY_RALT,
//     39           3A         3B       3C     3D      3E      3F
	KEY_SPACE, KEY_CAPSLOCK, KEY_F1, KEY_F1, KEY_F2, KEY_F3, KEY_F4,
//    40     41       42     43      44      45        46
	KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_SCROLLLOCK,
//     47       48       49         4A             4B
	KEY_HOME, KEY_UP, KEY_PAGEUP, KEY_KP_MINUS, KEY_LEFT,
//    4C        4D          4E          4F        50
	KEY_KP_5, KEY_RIGHT, KEY_KP_PLUS, KEY_END, KEY_DOWN,
//     51             52          53          54           55           56         57       58
	KEY_PAGEDOWN, KEY_INSERT, KEY_DELETE, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, KEY_F11, KEY_F12
};

void kbd::keyboardIntHandler(registers_t* r) {
    uint8_t data;
    ps2::readData(&data); // Read the data returned by the keyboard in PS/2 Controller data

    stdio::kprintf("KBD - data: %02x\n", data);
}