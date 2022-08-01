#include "ksysfuncs.h"

int main() {
    char inLineBuff[256]; // Max keyboard.h buffer is 256 so we set buffer to its max value.
    ksysfuncs::printStr("Write some text below::\n>");
    ksysfuncs::readln(inLineBuff);
    ksysfuncs::printf("You wrote: %s", inLineBuff);

    // sysfuncs::printf("SHELL - printf %d\n", 12);
    return 1;
}