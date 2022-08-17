#pragma once
#ifndef _MEMUTILS_H_
#define _MEMUTILS_H_

#include <stdint.h>

/**
 * @brief Memory management utilities
 * 
 */
namespace memutils
{
    /**
     * @brief Copy memory contents from a source pointer to a destination pointer memory
     * 
     * @param dst Destination pointer
     * @param src Source pointer
     * @param size Size to copy
     * @return void* Destination pointer result
     */
    void *memcpy(void *dst, const void *src, uint32_t size);

    /**
     * @brief Copy memory contents from a source pointer to a destination pointer memory using a default value
     * 
     * @param dst Destination pointer
     * @param src Source pointer
     * @param defVal Default value used when one of the bytes from the source being copied is null
     * @param size Size to copy
     * @return void* Destination pointer result
     */
    void *memcpy_def(void *dst, const void *src, unsigned char defVal, uint32_t size);

    /**
     * @brief Copy memory contents from a source pointer to a destination pointer memory in reverse order
     * 
     * @param dst Destination pointer
     * @param src Source pointer
     * @param size Size to copy
     * @return void* Destination pointer result
     */
    void *memcpy_r(void *dst, const void *src, uint32_t size);

    /**
     * @brief Set given source value in entire destination pointer memory slots
     * 
     * @param dst Destination pointer
     * @param src Source pointer
     * @param size Size to copy
     * @return void* Destination pointer result
     */
    void *memset(void *dst, uint32_t val, uint32_t size);

    /**
     * @brief Set given source value in entire destination pointer memory slots 2 bytes (short)
     * 
     * @param dst Destination pointer
     * @param src Source pointer
     * @param size Size to copy
     * @return void* Destination pointer result
     */
    void *memset_16(void *dest, uint16_t val, uint16_t len);

    /**
     * @brief Compare two memory values using a predetermined size
     * 
     * @param s1 Value 1 memory pointer
     * @param s2 Value 2 memory pointer
     * @param n Size being compared
     * @return int {< 0 Value 1 is less than value 2, > 0 Value 1 is greather than value 2, = 0 both values equals}
     */
    int memcmp(const void *s1, const void *s2, uint32_t n);

    /**
     * @brief Searches within the first num bytes of the block of memory pointed by ptr for the first occurrence of value 
     *        (interpreted as an unsigned char), and returns a pointer to it.
     * 
     * @param ptr       Pointer to the block of memory where the search is performed.
     * @param value     Value to be located. The value is passed as an int, but the function performs a byte per byte search using the unsigned char conversion of this value.
     * @param n         Number of bytes to be analyzed.
     * @return void*    A pointer to the first occurrence of value in the block of memory pointed by ptr.
     */
    void *memchr(const void *ptr, int value, uint32_t n);

    /**
     * @brief Print a memory dump
     * 
     * @param desc      Title of this memory dump
     * @param addr      Initial address of the memory region being dumped
     * @param len       Size of the memory dump being performed
     * @param perLine   The amount of bytes per line. (Min=4, Max=64, Default=16)
     */
    void memHexDump(const char *desc, const void *addr, const int len, int perLine);
}

#endif