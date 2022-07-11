#include "string.h"

int string::strstrip(char* s) {
    int pos = string::strlen(s);
    while (--pos >= 0)
        if (s[pos] == '\n')
            s[pos] = 0;
    return pos;
}

void string::reverse(char* s) {
    const int len = string::strlen(s) - 1;
    int i, j;
    for (i = 0, j = len; s[i] != s[j]; i++, j--) {
        char tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
    }
}

int string::strlen(char* s) {
    int i = 0;
    while (s[i] != 0)
        i++;
    return i;
}

int string::strlen(const char* s) {
    return string::strlen((char*)s);
}