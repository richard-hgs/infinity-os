#include "sysfuncs.h"

int main() {
    sysfuncs::printStr("SHELL - Hello World1.\n");
    sysfuncs::printf("SHELL - printf %d\n\0", 12);
    while(true){}
    return 0;
}