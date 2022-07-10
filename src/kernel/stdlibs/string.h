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
     * @param s 
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
}

#endif