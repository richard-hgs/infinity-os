#include "sysfuncs.h"

int main() {
    sysfuncs::printStr("SHELL - Hello World1.\n");
    sysfuncs::printf("SHELL - printf %d\n", 12);
    sysfuncs::exit(0);
    return 0;
}