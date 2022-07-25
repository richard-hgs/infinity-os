// libc 
#include <stdbool.h>
// cpu
#include "isr.h"
// stdio
#include "stdio.h"
// sys
#include "io.h"
#include "keyboard.h"

/**
 * @brief PS/2 Controller IO Ports
 * 
 */
#define IO_DATA         0x60                // Data Port                        - Read/Write
#define IO_CR_STATUS    0x64                // Command and Status Register Port - (Status=Read) (Command=Write)

/**
 * @brief PS/2 Controller Status Bits
 * Read status from IO_CR_STATUS(0x60)
 */
#define STATUS_OUT_BUFFER_STATUS    0x01      // Output buffer status - (0=Empty, 1=Full) - Must be set before attempting to read data from IO port 0x60
#define STATUS_IN_BUFFER_STATUS     0x02      // Input buffer status  - (0=Empty, 1=Full) - Must be clear before attempting to write data to IO port 0x60 or IO port 0x64
#define STATUS_SYSTEM_FLAG          0x04      // System Flag          - (0=Cleared on reset, 1=Set by firmware) - Is set (via. PS/2 Controller Configuration Byte) if the system passes self tests (POST)
#define STATUS_CMD_DATA             0x08      // Command/data         - (0=Data written to input buffer is data for PS/2 device, 1=Data written to input buffer is data for PS/2 controller command)
#define STATUS_4_RESERVED           0x10      // Reserved             - May be "keyboard lock" (more likely unused on modern systems)
#define STATUS_5_RESERVED           0x20      // Reserved             - May be "receive time-out" or "second PS/2 port output buffer full"
#define STATUS_TIMEOUT_ERROR        0x40      // Time-out error       - (0=No error, 1=Time-out error)
#define STATUS_PARITY_ERROR         0x80      // Parity error         - (0=No error, 1=Parity error)

/**
 * @brief PS/2 Controller Commands
 * 
 */
#define CTRL_CMD_READ_CONFIGURATION  0x20          // Read "byte 0" from internal RAM Controller Configuration Byte
#define CTRL_CMD_WRITE_CONFIGURATION 0x60          // Write next byte to "byte 0" of internal RAM
#define CTRL_CMD_PS2_PORT2_DISABLE   0xA7          // Disable seccond PS/2 port only if supported. If the controller is a "single channel" device, it will ignore the "command 0xA7"
#define CTRL_CMD_PS2_PORT2_ENABLE    0xA8          // Enable seccond PS/2 port only if supported.
#define CTRL_CMD_PS2_CTRL_PORT2_TEST 0xA9          // Test PS/2 Port 2
#define CTRL_CMD_PS2_CTRL_TEST       0xAA          // Test PS/2 Controller
#define CTRL_CMD_PS2_CTRL_PORT1_TEST 0xAB          // Test PS/2 Port 1
#define CTRL_CMD_PS2_PORT1_DISABLE   0xAD          // Disable first PS/2 port
#define CTRL_CMD_PS2_PORT1_ENABLE    0xAE          // Enable first PS/2 port

/**
 * @brief PS/2 Controller Configuration Bits
 * 
 */
#define CTRL_CONF_PS2_PORT1_INT_ENABLED             0x01    // First PS/2 port interrupt (1=Enabled, 0=Disabled)
#define CTRL_CONF_PS2_PORT2_INT_ENABLED             0x02    // Second PS/2 port interrupt (1=Enabled, 0=Disabled, only if 2 PS/2 ports supported)
#define CTRL_CONF_SYSTEM_FLAG                       0x04    // System Flag (1=System passed POST, 0=Your OS shouldn't be running)
#define CTRL_CONF_3_RESERVED                        0x08    // Should be zero
#define CTRL_CONF_PS2_PORT1_CLOCK_ENABLED           0x10    // First PS/2 port clock (1=Disabled, 0=Enabled)
#define CTRL_CONF_PS2_PORT2_CLOCK_ENABLED           0x20    // First PS/2 port clock (1=Disabled, 0=Enabled, only if 2 PS/2 ports supported)
#define CTRL_CONF_PS2_PORT1_TRANSLATION_ENABLED     0x40    // First PS/2 port translation (1=Enabled, 0=Disabled)
#define CTRL_CONF_7_RESERVED                        0x80    // Must be zero

/**
 * @brief PS/2 Controller Test result from data port
 * 
 */
#define CTRL_TEST_RESULT_SUCCESS                    0x55    // Test passed - success
#define CTRL_TEST_RESULT_FAILED                     0xFC    // Test failed - error

#define PORT_TEST_RESULT_SUCCESS                    0x00    // Test passed - success
#define PORT_TEST_RESULT_CLOCK_LINE_STUCK_LOW       0x01    // Test failed - Clock line stuck low
#define PORT_TEST_RESULT_CLOCK_LINE_STUCK_HIGH      0x02    // Test failed - Clock line stuck high
#define PORT_TEST_RESULT_DATA_LINE_STUCK_LOW        0x03    // Test failed - Data line stuck low
#define PORT_TEST_RESULT_DATA_LINE_STUCK_HIGH       0x04    // Test failed - Data line stuck high

uint8_t lastKbdData;

void keyboardInterruptHandler(registers_t* r) {
    if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
        lastKbdData = io::inb(IO_DATA);
        stdio::kprintf("keyboard %x\n", lastKbdData);
    }
}

uint8_t keyboard::install() {
    uint8_t data;
    bool isDualChannel; // true=PS/2 Is dual channel, false=Single chanel only

    // Determine if PS/2 Controller exists through ACPI Check bit 1 (value = 2, the "8042" flag) in the "IA PC Boot Architecture Flags" field at offset 109 in the Fixed ACPI Description Table (FADT). If this bit is clear, then there is no PS/2 Controller

    // Disable PS/2 ports to avoid PS/2 devices from sending data during Controller configuration
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_DISABLE);
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_DISABLE);

    isr::registerIsrHandler(IRQ1, keyboardInterruptHandler);  // Register Keyboard IRQ handler

    // Flush Controller Output Buffer to avoid data stuck in buffer
    io::inb(IO_DATA);                 // Read/Discard data from buffer

    // Change PS/2 Controller configuration
    io::outb(IO_CR_STATUS, CTRL_CMD_READ_CONFIGURATION);    // Request to controller to send controller configuration to data buffer
    // Read status byte               
    if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
        // Read configuration sent by controller to data buffer
        data = io::inb(IO_DATA);

        // stdio::kprintf("PS/2 - old config: %08b\n", data);

        // Enable first PS/2 port interrupt bit 0
        data |= CTRL_CONF_PS2_PORT2_INT_ENABLED | CTRL_CONF_PS2_PORT1_INT_ENABLED;

        // Write configuration
        io::outb(IO_CR_STATUS, CTRL_CMD_WRITE_CONFIGURATION);
        io::outb(IO_DATA, data);

        // Read PS/2 Controller configuration to check the new configuration
        io::outb(IO_CR_STATUS, CTRL_CMD_READ_CONFIGURATION);    // Request to controller to send controller configuration to data buffer
        if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
            // Read configuration sent by controller to data buffer
            data = io::inb(IO_DATA);
            stdio::kprintf("PS/2 - new config: %08b\n", data);
        }
    }
    
    // PS/2 Controller self test
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_CTRL_TEST);
    data = io::inb(IO_DATA);
    if (data == CTRL_TEST_RESULT_FAILED) { // Controller self test failed
        return PS2_ERROR_CONTROLLER_TEST;
    }

    // Check if PS/2 is dual channel by enabling port 2
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_ENABLE);      // Enalbe PS/2 Second port controller

    io::outb(IO_CR_STATUS, CTRL_CMD_READ_CONFIGURATION);    // Request to controller to send controller configuration to data buffer
    // Read status byte               
    if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
        // Read configuration sent by controller to data buffer
        data = io::inb(IO_DATA);

        // stdio::kprintf("PS/2 - config: %08b\n", data);

        if (!(data & CTRL_CONF_PS2_PORT2_CLOCK_ENABLED)) { // If this bit is clear it's a dual channel, if set isn't dual channel
            // Is dual channel
            isDualChannel = true;
            // stdio::kprintf("PS/2 - Is dual channel\n", data);
        }
    }

    // Disable PS/2 port 2 again to prevent interruptions
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_DISABLE);

    // Perform PS/2 port 1 test
    data = testPort(CTRL_CMD_PS2_CTRL_PORT1_TEST);
    if (data != PS2_NO_ERROR) { // Test failed
        return data; // Return error code
    }

    // Perform PS/2 port 2 test if supported
    if (isDualChannel) {
        data = testPort(CTRL_CMD_PS2_CTRL_PORT2_TEST);
        if (data != PS2_NO_ERROR) { // Test failed
            return data; // Return error code
        }
    }

    // Enable all PS/2 ports that exists
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_ENABLE);
    if (isDualChannel) {
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_ENABLE);
    }

    // Reset PS/2 Device 1
    io::outb(IO_CR_STATUS, 0xD0);
    io::outb(IO_DATA, 0xFF);

    // PS/2 Device reset
    if (isDualChannel) {
        // Reset PS/2 Device 2
        // Check if input buffer data needs to be read
        // io::outb(IO_CR_STATUS, 0xD4);
        // io::outb(IO_DATA, 0xFF);
        // data = io::inb(0xD3);
    }

    return PS2_NO_ERROR;
}

uint8_t keyboard::testPort(uint8_t port) {
    uint8_t data;
    io::outb(IO_CR_STATUS, port);
    if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
        data = io::inb(IO_DATA);
        if (data != PORT_TEST_RESULT_SUCCESS) {
            if (data == PORT_TEST_RESULT_CLOCK_LINE_STUCK_LOW) {
                return port == CTRL_CMD_PS2_CTRL_PORT1_TEST ? PS2_ERROR_PORT1_TEST_CLOCK_LINE_STUCK_LOW : PS2_ERROR_PORT2_TEST_CLOCK_LINE_STUCK_LOW;
            } else if (data == PORT_TEST_RESULT_CLOCK_LINE_STUCK_HIGH) {
                return port == CTRL_CMD_PS2_CTRL_PORT1_TEST ? PS2_ERROR_PORT1_TEST_CLOCK_LINE_STUCK_HIGH : PS2_ERROR_PORT2_TEST_CLOCK_LINE_STUCK_HIGH;
            } else if (data == PORT_TEST_RESULT_DATA_LINE_STUCK_LOW) {
                return port == CTRL_CMD_PS2_CTRL_PORT1_TEST ? PS2_ERROR_PORT1_TEST_DATA_LINE_STUCK_LOW : PS2_ERROR_PORT2_TEST_DATA_LINE_STUCK_LOW;
            } else if (data == PORT_TEST_RESULT_DATA_LINE_STUCK_HIGH) {
                return port == CTRL_CMD_PS2_CTRL_PORT1_TEST ? PS2_ERROR_PORT1_TEST_DATA_LINE_STUCK_HIGH : PS2_ERROR_PORT2_TEST_DATA_LINE_STUCK_HIGH;
            }
        }
    }
    return PS2_NO_ERROR;
}