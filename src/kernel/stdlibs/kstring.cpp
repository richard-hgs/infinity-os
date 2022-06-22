#include "kstring.h"

int kstring::strstrip(char *s)
{
	int pos = kstring::strlen(s);
	while(--pos >= 0)
		if(s[pos] == '\n')
			s[pos] = 0;
	return pos;
}


void kstring::reverse(char *s)
{
	const int len = kstring::strlen(s)-1;
	int i, j;
	for(i = 0, j = len; s[i] != s[j]; i++, j--) {
		char tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
	}
}

int kstring::strlen(char *s)
{
	int i = 0;
	while(s[i] != 0) i++;
	return i;
}

int kstring::strlen(const char *s)
{
	return kstring::strlen((char*) s);
}