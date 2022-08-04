#include "sysfuncs.h"
#include "string.h"

using namespace sysfuncs;

int getCmdNextArg(char *cmd, int startOffset, char *arg, int* endOffset);

int main() {
    char* cmd = (char*) malloc(256); // Max keyboard.h buffer is 256 so we set buffer to its max value.
    char* cmdArg = (char*) malloc(256);
    int argOffset;

    printf("INFINITY OS - Write some comand and press ENTER\n>");

    while(true) {
        argOffset = 0;
        // Move this process to waiting keyboard queue and wait until a keyboard ENTER key is pressed
        // then switch to my process again and continue execution.
        readln(cmd);

        // Get first argument from command
        getCmdNextArg(cmd, 0, cmdArg, &argOffset);

        // Compare all commands
        if (string::strcmp(cmdArg, "ps") == 0) {
            getCmdNextArg(cmd, argOffset, cmdArg, &argOffset);
            if (string::strcmp(cmdArg, "list") == 0) {
                printProcessList();
            }
        } else if (string::strcmp(cmdArg, "help") == 0) {
            printf("----------- COMMANDS -----------\n");
            printf("help - Show information about the available commands;\n");
            printf("ps   - Process Commands;\n");
            printf("   list - List all processes running;");
        } else {
            printf("Unknow command. Use help command to list available commands.");
        }
        printf("\n>");
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