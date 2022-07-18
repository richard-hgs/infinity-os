#include "string.h"

int string::strstrip(char* s) {
    int pos = strlen(s);
    while (--pos >= 0)
        if (s[pos] == '\n')
            s[pos] = 0;
    return pos;
}

void string::reverse(char* s) {
    const int len = strlen(s) - 1;
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
    return strlen((char*)s);
}

int string::strcmp(const char* s1, const char* s2) {
   while (*s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
      s1++;
      s2++;
   }
   return *s1 - *s2;
}

int string::strncmp(const char* s1, const char* s2, int size) {
    while (size != 0 && *s1 != '\0' && *s2 != '\0'  && *s1 == *s2) {
      s1++;
      s2++;
      size--;
   }
   return size == 0 ? 0 : (*s1 - *s2);
}