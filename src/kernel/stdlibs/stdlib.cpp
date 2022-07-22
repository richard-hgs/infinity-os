// libc
#include <stdint.h>
// stdlibs
#include "string.h"
#include "stdlib.h"
// drivers
#include "vga.h" // Debug only

#define X86_DIGITS 32
#define X64_DIGITS 64

#define VA_STR_BUFFER_SIZE 64

unsigned long stdlib::ultoa(unsigned long value, unsigned char radix, char *str)  {
	unsigned long written = 0;
	unsigned int index;
	unsigned char ch;
	unsigned char buffer[X86_DIGITS];  /* space for X86_DIGITS + '\0' */

	index = X86_DIGITS;

	do {
		ch = '0' + (value % radix);
		if ( ch > (unsigned char)'9') {
			ch += 'a' - '9' - 1;
		}
        buffer[--index] = ch;
        value /= radix;
    } while (value != 0);

	do {
		*str++ = buffer[index++];
		written++;
    } while (index < X86_DIGITS);

    *str = 0;  /* string terminator */

	return written;
}

long stdlib::ltoa(long value, unsigned char radix, char* str) {
	if (value < 0 && radix == 10) {
		*str++ = '-';
		value = -value;
	}

	return static_cast<long>(stdlib::ultoa((unsigned long) value, radix, str));
}

int stdlib::itoa(int value, unsigned char radix, char *str) {
  return static_cast<int>(stdlib::ltoa(value, radix, str));
}

void stdlib::uitoa(unsigned int value, unsigned char radix, char *str) {
  stdlib::ultoa((unsigned long) value, radix, str);
}

// void u64toa(uint64_t value, char* str, unsigned char radix) {
// 	unsigned int index;
// 	unsigned char ch;
// 	unsigned char buffer[X64_DIGITS];  /* space for X64_DIGITS + '\0' */

// 	index = X64_DIGITS;

// 	do {
// 		ch = '0' + (value % radix);
// 		if ( ch > (unsigned char)'9') {
// 			ch += 'a' - '9' - 1;
// 		}
//       buffer[--index] = ch;
//       value /= radix;
//     } while (value != 0);

// 	do {
//       *str++ = buffer[index++];
//     } while (index < X64_DIGITS);

//     *str = 0;  /* string terminator */
// }

int	stdlib::atoi(char *str) {
	int neg;
	int num;
	int i;

	i = 0;
	neg = 1;
	num = 0;
	while (str[i] <= ' ') {
		i++;
	}
	if (str[i] == '-' || str[i] == '+') {
		if (str[i] == '-') {
			neg *= -1;
		}
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9') {
		num = num * 10 + (str[i] - 48);
		i++;
	}
	return (num * neg);
}


int stdlib::va_stringf(char *strDest, const char* strFormat, va_list list) {
	int i;
	int size = 0;
	char tempstr[VA_STR_BUFFER_SIZE];
    char *pStr;
	char tempchar;
	int tempint;
	int j;
	int leadCount = 0;
	int strFormatLen = string::strlen(strFormat);

	for(i = 0; i < strFormatLen; i++) {
		if(*(strFormat+i) == '%') {
			i++;

			if (*(strFormat+i) == '0') { // Leading zeros format found
				j=0; // Reset one int var to use in leadNumStr offset index
				i++;
				for (; i < strFormatLen && *(strFormat+i) >= '0' && *(strFormat+i) <= '9'; i++) { // While numeric chars found read them
					if (j < VA_STR_BUFFER_SIZE - 2) {
						*(tempstr+j) = *(strFormat+i);
					}
					j++;
				}
				tempstr[j] = '\0';
				leadCount = atoi(tempstr);

				for (; leadCount > 0; leadCount--) { // Adds zeroes to the string before the format
					*(strDest + size) = '0';
					size++;
					if (leadCount % 10 == 0) {
						*(strDest + size) = ' ';
						size++;
					}
				}
			}


			switch(*(strFormat+i)) {
				case 's':
					pStr = va_arg(list, char*);
					for(j = 0; j < string::strlen(pStr); j++) {
						*(strDest + size) = *(pStr + j);
						size++;
					}
					break;
				case 'c':
					tempchar = va_arg(list, int);
					*(strDest + size) = tempchar;
					size++;
					break;
				case 'd':
					tempint = va_arg(list, int);
					stdlib::itoa(tempint, 10, tempstr);
					for(j = 0; j < string::strlen(tempstr); j++) {
                        *(strDest + size) = *(tempstr + j);
                        size++;
                    }
                    break;
				case 'x':
					tempint = va_arg(list, int);
					stdlib::itoa(tempint, 16, tempstr);
					for(j = 0; j < string::strlen(tempstr); j++) {
                        *(strDest + size) = *(tempstr + j);
                        size++;
                    }
                    break;
				case 'b':
					tempint = va_arg(list, int);
					stdlib::itoa(tempint, 2, tempstr);
                    for(j = 0; j < string::strlen(tempstr); j++) {
						*(strDest + size) = *(tempstr + j);
						size++;
					}
                    break;
				default:
					*(strDest + size) = *(strFormat + i);
					size++;
					break;
			}
		} else {
			// No format found print char by char
			*(strDest + size) = *(strFormat + i);
			size++;
		}
	}

	// End of string NULL CHAR
    strDest[size] = 0;

	return size;
}