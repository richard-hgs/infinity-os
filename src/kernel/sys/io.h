#pragma once
#ifndef _IO_H_
#define _IO_H_

// libc
#include <stdint.h>

/**
 * @brief Kernel io operations
 * - Read/write data in hardware port address offset in RAM memory
 * 
 */
namespace io {
    /**
     * @brief Read byte from given port address.
     * 
     * @param port 
     * @return uint8_t Byte value
     */
    uint8_t inb(uint16_t port);


    /**
     * @brief Write byte to given port.
     * 
     * @param port  Port address to write
     * @param value Byte to write
     */
    void outb(uint16_t port, uint8_t value);


    /**
     * @brief Read short value from given port address.
     * 
     * @param port Port address to read
     * @return uint16_t Value readed
     */
    uint16_t inw(uint16_t port);

    /**
     * @brief Write short value to given port address.
     * 
     * @param port Port address to write
     * @param value Short value to write
     */
    void outw(uint16_t port, uint16_t value);

    /**
     * @brief IO_WAIT Wait a very small amount of time (1 to 4 microseconds, generally). 
     * Useful for implementing a small delay for PIC remapping on old hardware or generally as a simple but imprecise wait.
     * 
     */
    void wait();
}
#endif