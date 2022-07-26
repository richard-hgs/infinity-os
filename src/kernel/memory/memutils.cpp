// stdlibs
#include "stdlib.h"
#include "stdio.h"
#include "memutils.h"

void *memutils::memcpy(void *dst, const void *src, uint32_t size) {
    uint32_t *wdst = (uint32_t *) dst;
    const uint32_t *wsrc = (uint32_t *) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t) wdst & 3) && !((uint32_t) wsrc & 3)) {
        while (size > 3) {
            *wdst++ = *wsrc++;
            size -= 4;
        }
    }

    unsigned char *cdst = (unsigned char *) wdst;
    unsigned char *csrc = (unsigned char *) wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--) {
        *cdst++ = *csrc++;
    }
    return dst;
}

void *memutils::memcpy_def(void *dst, const void *src, unsigned char defVal, uint32_t size) {
    uint32_t *wdst = (uint32_t *) dst;
    const uint32_t *wsrc = (uint32_t *) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t) wdst & 3) && !((uint32_t) wsrc & 3)) {
        while (size > 3) {
            *wdst++ = *wsrc++;
            size -= 4;
        }
    }

    unsigned char *cdst = (unsigned char *) wdst;
    unsigned char *csrc = (unsigned char *) wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--) {
        unsigned char sourceVal = *csrc++;
        if (sourceVal == 0) {
            sourceVal = defVal;
        }
        *cdst++ = sourceVal;
    }
    return dst;
}

void *memutils::memcpy_r(void *dst, const void *src, uint32_t size) {
    uint32_t *wdst = (uint32_t *) dst;
    const uint32_t *wsrc = (uint32_t *) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t) wdst & 3) && !((uint32_t) wsrc & 3)) {
        while (size > 3) {
            *wdst-- = *wsrc--;
            size -= 4;
        }
    }

    unsigned char *cdst = (unsigned char *) wdst;
    unsigned char *csrc = (unsigned char *) wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--) {
        *cdst-- = *csrc--;
    }
    return dst;
}

void *memutils::memcpy_16(void *dst, const void *src, uint32_t size) {
    uint32_t *wdst = (uint32_t *) dst;
    const uint32_t *wsrc = (uint32_t *) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t) wdst & 3) && !((uint32_t) wsrc & 3)) {
        while (size > 3) {
            *wdst++ = *wsrc++;
            size -= 4;
        }
    }

    uint16_t *cdst = (uint16_t *) wdst;
    uint16_t *csrc = (uint16_t *) wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--) {
        *cdst++ = *csrc++;
    }
    return dst;
}

void *memutils::memcpy_16_def(void *dst, const void *src, uint16_t defVal, uint32_t size) {
    uint32_t *wdst = (uint32_t *) dst;
    const uint32_t *wsrc = (uint32_t *) src;

    // word per word copy if both addresses aligned
    if (!((uint32_t) wdst & 3) && !((uint32_t) wsrc & 3)) {
        while (size > 3) {

            *wdst++ = *wsrc++;
            size -= 4;
        }
    }

    uint16_t *cdst = (uint16_t *) wdst;
    uint16_t *csrc = (uint16_t *) wsrc;

    // byte per byte for last bytes (or not aligned)
    while (size--) {
        uint16_t sourceVal = *csrc++;
        if (sourceVal == 0) {
            sourceVal = defVal;
        }
        *cdst++ = sourceVal;
    }
    return dst;
}

void *memutils::memset(void *dst, uint32_t val, uint32_t size) {
    // build 8 bits and 32 bits values
    uint8_t byte = (uint8_t) (val & 0xFF);
    uint32_t word = (val << 24) | (val << 16) | (val << 8) | val;

    // word per word if address aligned
    uint32_t *wdst = (uint32_t *) dst;

    if ((((uint32_t) dst) & 0x3) == 0) {
        while (size > 3) {
            *wdst++ = word;
            size -= 4;
        }
    }

    // byte per byte for last bytes (or not aligned)
    char *cdst = (char *) wdst;

    while (size--) {
        *cdst++ = byte;
    }

    return dst;
}

void *memutils::memset_16(void *dest, uint16_t val, uint16_t len) {
    uint16_t *ptr = (uint16_t *) dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

void *memset_16_safe(void *dest, uint16_t val, uint16_t len) {
    uint16_t *buf = (uint16_t *) dest;
    union {
        uint8_t d8[2];
        uint16_t d16;
    } u16 = {.d16 = val};

    while (len--) {
        *buf++ = u16.d8[0];
        *buf++ = u16.d8[1];
    }
    return dest;
}

int memutils::memcmp(const void *s1, const void *s2, uint32_t n) {
    const uint8_t *cs1 = (uint8_t *) s1;
    const uint8_t *cs2 = (uint8_t *) s2;

    while (n > 0) {
        if (*cs1++ != *cs2++)
            return (*--cs1 - *--cs2);
        n--;
    }
    return 0;
}

void *memutils::memchr(const void *ptr, int value, uint32_t n) {
    const unsigned char *src = (const unsigned char *) ptr;

    while (n-- > 0) {
        if (*src == value)
            return (void *) src;
        src++;
    }
    return NULL;
}

void memutils::memHexDump(const char *desc, const void *addr, const int len, int perLine) {
    // Silently ignore silly per-line values.
    if (perLine < 4 || perLine > 64) {
        perLine = 16;
    }

    int i;
    unsigned char buff[perLine + 1];
    const unsigned char *pc = (const unsigned char *) addr;

    // Output description if given.
    if (desc != NULL)
        stdio::kprintf("%s:\n", desc);

    // Length checks.
    if (len == 0) {
        stdio::kprintf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        stdio::kprintf("  NEGATIVE LENGTH: %d\n", len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of perLine means new or first line (with line offset).

        if ((i % perLine) == 0) {
            // Only print previous-line ASCII buffer for lines beyond first.
            if (i != 0) {
                stdio::kprintf("  %s\n", buff);
            }

            // Output the offset of current line.
            stdio::kprintf("  %04x ", i & 0xFFFF);
        }

        // Now the hex code for the specific character.
        stdio::kprintf(" %02x", pc[i] & 0xFF);

        // And buffer a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) { // isprint() may be better.
            buff[i % perLine] = '.';
        } else {
            buff[i % perLine] = pc[i];
        }
        buff[(i % perLine) + 1] = '\0';
    }

    // Pad out last line if not exactly perLine characters.
    while ((i % perLine) != 0) {
        stdio::kprintf("   ");
        i++;
    }

    // And print the final ASCII buffer.

    stdio::kprintf("  %s\n", buff);
}