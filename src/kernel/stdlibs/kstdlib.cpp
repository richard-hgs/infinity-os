#include <stdint.h>
#include "kstring.h"
#include "kstdlib.h"

#define X86_DIGITS 32
#define X64_DIGITS 64

#define VA_STR_BUFFER_SIZE 1024

unsigned long kstdlib::ultoa(unsigned long value, unsigned char radix, char* str) 
{
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

long kstdlib::ltoa(long value, unsigned char radix, char* str)
{
	if (value < 0 && radix == 10) {
		*str++ = '-';
		value = -value;
	}

	return static_cast<long>(kstdlib::ultoa((unsigned long) value, radix, str));
}

int kstdlib::itoa(int value, unsigned char radix, char *str) 
{
  return static_cast<int>(kstdlib::ltoa(value, radix, str));
}

void kstdlib::uitoa(unsigned int value, unsigned char radix, char *str) 
{
  kstdlib::ultoa((unsigned long) value, radix, str);
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

int	kstdlib::atoi(char *str) 
{
	int neg;
	int num;
	int i;

	i = 0;
	neg = 1;
	num = 0;
	while (str[i] <= ' ')
		i++;
	if (str[i] == '-' || str[i] == '+')
	{
		if (str[i] == '-')
		{
			neg *= -1;
		}
		i++;
	}
	while (str[i] >= '0' && str[i] <= '9')
	{
		num = num * 10 + (str[i] - 48);
		i++;
	}
	return (num * neg);
}


int kstdlib::va_stringf(char *strDest, const char* strFormat, va_list list)
{
	int i;
	int size = 0;
	char tempstr[VA_STR_BUFFER_SIZE];
    char *pStr;
	char tempchar;
	int tempint;
	int j;

	for(i = 0; i < kstring::strlen(strFormat); i++)
	{
		if(*(strFormat+i) == '%')
		{
			i++;
			switch(*(strFormat+i))
			{
				case 's':
					pStr = va_arg(list, char*);
					for(j = 0; j < kstring::strlen(pStr); j++)
					{
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
					kstdlib::itoa(tempint, 10, tempstr);
					for(j = 0; j < kstring::strlen(tempstr); j++)
                    {
                        *(strDest + size) = *(tempstr + j);
                        size++;
                    }
                    break;
				case 'x':
					tempint = va_arg(list, int);
					kstdlib::itoa(tempint, 16, tempstr);
					for(j = 0; j < kstring::strlen(tempstr); j++)
                    {
                        *(strDest + size) = *(tempstr + j);
                        size++;
                    }
                    break;
				case 'b':
					tempint = va_arg(list, int);
					kstdlib::itoa(tempint, 2, tempstr);
                    for(j = 0; j < kstring::strlen(tempstr); j++)
                        {
                            *(strDest + size) = *(tempstr + j);
                            size++;
                        }
                    break;
				default:
					*(strDest + size) = *(strFormat + i);
					size++;
					break;
			}
		}
		else
		{
			*(strDest + size) = *(strFormat + i);
			size++;
		}
	}

    strDest[size] = 0;

	return size;
}