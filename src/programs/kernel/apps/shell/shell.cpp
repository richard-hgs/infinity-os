#include "ksysfuncs.h"

using namespace ksysfuncs;

int main() {
    char* inLineBuff = (char*) malloc(256); // Max keyboard.h buffer is 256 so we set buffer to its max value.

    print("Write some text below::\n>");
    readln(inLineBuff);
    printf("You wrote: %s\n", inLineBuff);

    free(inLineBuff);

    printProcessList();

    // sysfuncs::printf("SHELL - printf %d\n", 12);
    return 1;
}