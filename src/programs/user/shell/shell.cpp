#include <stdbool.h>
#include "sysfuncs.h"
#include "string.h"

using namespace sysfuncs;

int getCmdNextArg(char *cmd, int startOffset, char *arg, int* endOffset);

int main() {
    char* cmd = (char*) malloc(256);    // Max keyboard.h buffer is 256 so we set buffer to its max value.
    char* cmdArg = (char*) malloc(256); 
    int argOffset;
    bool eocLineBreak; // Adds line break to the end of command

    // printf("\033[0");
    printf("INFINITY OS - Write some comand and press ENTER\n>");

    while(true) {
        argOffset = 0;
        eocLineBreak = true;
        // Move this process to waiting keyboard queue and wait until a keyboard ENTER key is pressed
        // then move this process to the ready queue and wait until it is executed again.
        readln(cmd);

        // Get first argument from command
        getCmdNextArg(cmd, 0, cmdArg, &argOffset);

        // Compare all commands
        if (string::strcmp(cmdArg, "ps") == 0) {            // PS - Process commands
            getCmdNextArg(cmd, argOffset, cmdArg, &argOffset);
            if (string::strcmp(cmdArg, "list") == 0) {
                printProcessList();
            }
        } if (string::strcmp(cmdArg, "clear") == 0) {       // VGA - Clear screen content
            clearScreen();
            eocLineBreak = false;
        } else if (string::strcmp(cmdArg, "help") == 0) {   // HELP - Show all available commands
            printf("----------- COMMANDS -----------\n");
            printf("help  - Show information about the available commands;\n");
            printf("ps    - Process Commands;\n");
            printf("   list - List all processes running;");
            printf("clear - Wipe text on the screen, also reset the cursor position;");
        } else {
            printf("\"%s\" command not found.", cmdArg);
        }
        if (eocLineBreak) {
            printf("\n");
        }
        printf(">");
    }

    free(cmd);
    free(cmdArg);

    return 1;
}

int getCmdNextArg(char* cmd, int startOffset, char* arg, int* endOffset) {
    int length = 0;

    cmd += startOffset;
    
    while(*cmd != '\0' && (length == 0 || *cmd != ' ')) {
        if (*cmd != ' ') {
            length++;
            *arg++ = *cmd++;
        } else {
            cmd++;
        }
        (*endOffset)++;
    }

    *arg++ = '\0';

    return length;
}

/*
 *   - \033 - (ESC) Escape Sequences:
 *         - [0 - Clear vga screen and set cursor to position col:0, row:0;
 *         - [1 - Set cursor position (E.q: setCursorPosition(col, row));
 *             - [00[00 - 1º Column, 2º Row
 */
/*
if (*str == '\033') { // Terminal Escape sequence
    str++;
    if (*str == '[') { // Indicates that now a scape function will begin
        str++;
        if (*str == '0') { // CLEAR_SCREEN(0) - Escape function number
            str++;
            clearScreen(foreColor, bgColor, false);
            // Reset vga memory offset
            cursorOffset = 0;
            video = (volatile uint16_t*) vgaAddress + cursorOffset;
        } else if (*str == '1') { // SET_CURSOR_POSITION(1) - Escape function number
            str++;
            int col = 0;
            int row = 0;
            for (i=0; i<2 && str[0] != '\0' && str[1] >= '0' && str[1] <= '9' && str[2] >= '0' && str[2] <= '9'; i++) {
                if (*str++ == '[') {
                    tmpBuff[0] = *str++;
                    tmpBuff[1] = *str++;
                    tmpBuff[2] = '\0';
                    (i == 0 ? col : row) = stdlib::atoi(tmpBuff);
                }
            }
            // Set cursor position
            setCursorPosition(col, row);
            // Reset vga memory offset
            cursorOffset = GET_SCREEN_OFFSET_POS(col, row);
            video = (volatile uint16_t*) vgaAddress + cursorOffset;
        }
    }
}
*/