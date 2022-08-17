#pragma once
#ifndef _STRING_H_
#define _STRING_H_

namespace string {

    /**
     * @brief Strip off new line from string.
     *
     * @param s String to strip new line from
     * @return int New string length and also the offset where the strip was performed in string
     */
    int strstrip(char *s);

    /**
     * @brief  Reverse a string in place.
     *
     * @param s String to be reversed
     */
    void reverse(char *s);

    /**
     * @brief Calculate string length
     *
     * @param s     String to retrieve length
     * @return int  The length of the string
     */
    int strlen(char *s);

    /**
     * @brief Calculate string length
     *
     * @param s     String to retrieve length
     * @return int  The length of the string
     */
    int strlen(const char *s);

    /**
     * @brief Compare two strings fully
     *
     * @param s1     String1 to be compared
     * @param s2     String2 to be compared
     * @return int   Compare result
     *
     * if Return value < 0 then it indicates str1 is less than str2.
     * if Return value > 0 then it indicates str2 is less than str1.
     * if Return value = 0 then it indicates str1 is equal to str2.
     */
    int strcmp(const char *s1, const char *s2);

    /**
     * @brief Compare two strings fully or partially
     *
     * @param s1    String1 to be compared
     * @param s2    String2 to be compared
     * @param size  Max char length to compare
     * @return int  Compare result
     *
     * if Return value < 0 then it indicates str1 is less than str2.
     * if Return value > 0 then it indicates str2 is less than str1.
     * if Return value = 0 then it indicates str1 is equal to str2.
     */
    int strncmp(const char *s1, const char *s2, int size);

    /**
     * @brief Copy source string to the destination string
     *
     * @param dest      Destination char ptr that will receive the copy
     * @param src       Source that is being copied
     * @return char*    The destination with copied data
     */
    char *strcpy(char *dest, const char *src);

    /**
     * @brief Read next argument from command string
     * 
     * @param cmd           Command to read argument from
     * @param startOffset   The base offset where to start looking
     * @param arg           Return the argument found
     * @param endOffset     The end offset from argument found
     * @return int 
     */
    int readNextArg(char *cmd, int startOffset, char *arg, int* endOffset);
} // namespace string

#endif