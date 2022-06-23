#pragma once
#ifndef _STDLIB_H_
#define _STDLIB_H_

#include <stdarg.h>

/**
 * @brief Kernel standard libraries
 * - ultoa
 * - ltoa
 * - itoa
 * - uitoa
 * - atoi
 * - va_stringf
 */
namespace stdlib {

    /**
     * @brief Convert unsigned long value to string using radix numeric base
     * 
     * @param value unsigned long value to be converted
     * @param radix Radix numeric base (E.g 10 = Decimal, 2 = Binary ...)
     * @param str Buffer that will store the converted value
     * @return unsigned long Buffer written length
     */
    unsigned long ultoa(unsigned long value, unsigned char radix, char* str);

    /**
     * @brief Convert long value to string using radix numeric base
     * 
     * @param value long value to be converted
     * @param radix Radix numeric base (E.g 10 = Decimal, 2 = Binary ...)
     * @param str Buffer that will store the converted value
     * @return long Buffer written length
     */
    long ltoa(long value, unsigned char radix, char* str);

    /**
     * @brief Convert int value to string using radix numeric base
     * 
     * @param value int value to be converted
     * @param radix Radix numeric base (E.g 10 = Decimal, 2 = Binary ...)
     * @param str Buffer that will store the converted value
     * @return int Buffer written length
     */
    int itoa(int value, unsigned char radix, char *str);

    /**
     * @brief Convert unsigned int value to string using radix numeric base
     * 
     * @param value unsigned int value to be converted
     * @param radix Radix numeric base (E.g 10 = Decimal, 2 = Binary ...)
     * @param str Buffer that will store the converted value
     */
    void uitoa(unsigned int value, unsigned char radix, char *str);

    /**
     * @brief Parse string digits to int representation
     * 
     * @param str   String to be converted
     * @return int  The number converted
     */
    int	atoi(char *str);

    /**
     * @brief Format a string using va_args
     * 
     * @param strDest       Destination string buffer to store formatted string
     * @param strFormat     Source string to format
     * @param list          Infinite arguments
     * @return int          The length of the formatted string
     */
    int va_stringf(char *strDest, const char *strFormat, va_list list);
}

#endif