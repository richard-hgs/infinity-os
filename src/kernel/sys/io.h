#pragma once
#ifndef _IO_H_
#define _IO_H_

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
     * @return unsigned char Byte value
     */
    unsigned char inb(unsigned short port);


    /**
     * @brief Write byte to given port.
     * 
     * @param port  Port address to write
     * @param value Byte to write
     */
    void outb(unsigned short port, unsigned char value);


    /**
     * @brief Read short value from given port address.
     * 
     * @param port Port address to read
     * @return unsigned short Value readed
     */
    unsigned short inw(unsigned short port);

    /**
     * @brief Write short value to given port address.
     * 
     * @param port Port address to write
     * @param value Short value to write
     */
    void outw(unsigned short port, unsigned short value);
}
#endif