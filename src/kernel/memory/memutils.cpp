
#include "memutils.h"

void *memutils::memcpy(void *dst, const void *src, uint32_t size)
{
    uint32_t *wdst = (uint32_t*) dst;
    const uint32_t *wsrc = (uint32_t*) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t)wdst & 3) && !((uint32_t)wsrc & 3))
    {
        while (size > 3)
        {
            *wdst++ = *wsrc++;
            size -= 4;
        }
    }

    unsigned char *cdst = (unsigned char *)wdst;
    unsigned char *csrc = (unsigned char *)wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--)
    {
        *cdst++ = *csrc++;
    }
    return dst;
}

void *memutils::memcpy_def(void *dst, const void *src, unsigned char defVal, uint32_t size)
{
    uint32_t *wdst = (uint32_t*) dst;
    const uint32_t *wsrc = (uint32_t*) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t)wdst & 3) && !((uint32_t)wsrc & 3))
    {
        while (size > 3)
        {
            *wdst++ = *wsrc++;
            size -= 4;
        }
    }

    unsigned char *cdst = (unsigned char *)wdst;
    unsigned char *csrc = (unsigned char *)wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--)
    {
        unsigned char sourceVal = *csrc++;
        if (sourceVal == 0)
        {
            sourceVal = defVal;
        }
        *cdst++ = sourceVal;
    }
    return dst;
}

void *memutils::memcpy_r(void *dst, const void *src, uint32_t size)
{
    uint32_t *wdst = (uint32_t*) dst;
    const uint32_t *wsrc = (uint32_t*) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t)wdst & 3) && !((uint32_t)wsrc & 3))
    {
        while (size > 3)
        {
            *wdst-- = *wsrc--;
            size -= 4;
        }
    }

    unsigned char *cdst = (unsigned char *)wdst;
    unsigned char *csrc = (unsigned char *)wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--)
    {
        *cdst-- = *csrc--;
    }
    return dst;
}

void *memutils::memcpy_16(void *dst, const void *src, uint32_t size)
{
    uint32_t *wdst = (uint32_t*) dst;
    const uint32_t *wsrc = (uint32_t*) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t)wdst & 3) && !((uint32_t)wsrc & 3))
    {
        while (size > 3)
        {
            *wdst++ = *wsrc++;
            size -= 4;
        }
    }

    uint16_t *cdst = (uint16_t *)wdst;
    uint16_t *csrc = (uint16_t *)wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--)
    {
        *cdst++ = *csrc++;
    }
    return dst;
}

void *memutils::memcpy_16_def(void *dst, const void *src, uint16_t defVal, uint32_t size)
{
    uint32_t *wdst = (uint32_t*) dst;
    const uint32_t *wsrc = (uint32_t*) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t)wdst & 3) && !((uint32_t)wsrc & 3))
    {
        while (size > 3)
        {

            *wdst++ = *wsrc++;
            size -= 4;
        }
    }

    uint16_t *cdst = (uint16_t *)wdst;
    uint16_t *csrc = (uint16_t *)wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--)
    {
        uint16_t sourceVal = *csrc++;
        if (sourceVal == 0)
        {
            sourceVal = defVal;
        }
        *cdst++ = sourceVal;
    }
    return dst;
}

void *memutils::memset(void *dst, uint32_t val, uint32_t size)
{
    // build 8 bits and 32 bits values
    uint8_t byte = (uint8_t)(val & 0xFF);
    uint32_t word = (val << 24) | (val << 16) | (val << 8) | val;

    // word per word if address aligned
    uint32_t *wdst = (uint32_t *)dst;

    if ((((uint32_t)dst) & 0x3) == 0)
    {
        while (size > 3)
        {
            *wdst++ = word;
            size -= 4;
        }
    }

    // byte per byte for last bytes (or not aligned)
    char *cdst = (char *)wdst;

    while (size--)
    {
        *cdst++ = byte;
    }

    return dst;
}

void *memutils::memset_16(void *dest, uint16_t val, uint16_t len)
{
    uint16_t *ptr = (uint16_t *) dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

void *memset_16_safe(void *dest, uint16_t val, uint16_t len)
{
    uint16_t *buf = (uint16_t *) dest;
    union 
    {
        uint8_t d8[2];
        uint16_t d16;
    } u16 = {.d16 = val};

    while(len--) 
    {
        *buf++ = u16.d8[0];
        *buf++ = u16.d8[1];
    }
    return dest;
}

int memutils::memcmp(const void *s1, const void *s2, uint32_t n)
{
    const uint8_t *cs1 = (uint8_t*) s1;
    const uint8_t *cs2 = (uint8_t*) s2;

    while (n > 0)
    {
        if (*cs1++ != *cs2++)
            return (*--cs1 - *--cs2);
        n--;
    }
    return 0;
}