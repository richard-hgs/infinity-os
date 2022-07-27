// libc 
#include <stdbool.h>
// cpu
#include "isr.h"
// stdio
#include "stdio.h" // Debug
// memory
#include "memutils.h"
// sys
#include "io.h"
#include "ps2.h"

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
#define CTRL_CMD_READ_CONFIGURATION  0x20           // Read "byte 0" from internal RAM Controller Configuration Byte
#define CTRL_CMD_WRITE_CONFIGURATION 0x60           // Write next byte to "byte 0" of internal RAM
#define CTRL_CMD_PS2_PORT2_DISABLE   0xA7           // Disable seccond PS/2 port only if supported. If the controller is a "single channel" device, it will ignore the "command 0xA7"
#define CTRL_CMD_PS2_PORT2_ENABLE    0xA8           // Enable seccond PS/2 port only if supported.
#define CTRL_CMD_PS2_CTRL_PORT2_TEST 0xA9           // Test PS/2 Port 2
#define CTRL_CMD_PS2_CTRL_TEST       0xAA           // Test PS/2 Controller
#define CTRL_CMD_PS2_CTRL_PORT1_TEST 0xAB           // Test PS/2 Port 1
#define CTRL_CMD_PS2_PORT1_DISABLE   0xAD           // Disable first PS/2 port
#define CTRL_CMD_PS2_PORT1_ENABLE    0xAE           // Enable first PS/2 port
#define CTRL_CMD_PS2_PORT1_IN_WRITE  0xD0           // Write next byte to first  PS/2 port input buffer
#define CTRL_CMD_PS2_PORT2_IN_WRITE  0xD4           // Write next byte to second PS/2 port input buffer (only if 2 PS/2 ports supported)
#define CTRL_CMD_PS2_RESET           0xFF           // Reset and start self-test 0xAA=(self-test passed), 0xFC or 0xFD=(self test failed), or 0xFE=(Resend)

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

#define DEVICE_CMD_DISABLE_SCANNING                 0xF5    // Disable scanning (If Keyboard=won't send scan codes, If Mouse=Disable Data Reporting)
#define DEVICE_CMD_IDENTIFY                         0xF2    // Identify Device Type

#define DEVICE_RESP_ERROR                           0x00    // Error or internal buffer overrun
#define DEVICE_RESP_ACK                             0xFA    // Command acknowledged (ACK)
#define DEVICE_RESP_SELF_TEST_SUCCESS               0xAA    // Self test passed (sent after "0xFF (reset)" command or keyboard power up)

/**
 * @brief PS/2 Device types
 * 
 */
// NONE=Ancient AT keyboard with translation enabled in the PS/Controller (not possible for the second PS/2 port)
#define DEVICE_TYPE_STANDARD_MOUSE                  0x00    // Standard PS/2 mouse
#define DEVICE_TYPE_MOUSE_WITH_SCROLL_WHEEL         0x03    // Mouse with scroll wheel
#define DEVICE_TYPE_5_BUTTON_MOUSE                  0x04    // 5-button mouse
#define DEVICE_TYPE_MF2_KEYBOARD_1_1                0xAB    // ‾|--> MF2 keyboard with translation enabled in the   
#define DEVICE_TYPE_MF2_KEYBOARD_1_2                0x41    // _|    PS/Controller (not possible for the second PS/2 port)
#define DEVICE_TYPE_MF2_KEYBOARD_2_1                0xAB    // ‾|--> MF2 keyboard with translation enabled in the   
#define DEVICE_TYPE_MF2_KEYBOARD_2_2                0xC1    // _|    PS/Controller (not possible for the second PS/2 port)
#define DEVICE_TYPE_MF2_KEYBOARD_3_1                0xAB    // ‾|--> MF2 keyboard  
#define DEVICE_TYPE_MF2_KEYBOARD_3_2                0x83    // _|


#define PS2_PORTS_DATA_BUFFER_SIZE 10

unsigned char ps2Port1DataBuffer[PS2_PORTS_DATA_BUFFER_SIZE];
unsigned char ps2Port2DataBuffer[PS2_PORTS_DATA_BUFFER_SIZE];

uint8_t ps2Port1DataLength = 0;
uint8_t ps2Port2DataLength = 0;

void ps2Port1IntHandler(registers_t* r) {
    uint8_t data;
    if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
        data = io::inb(IO_DATA);
        if (ps2Port1DataLength < PS2_PORTS_DATA_BUFFER_SIZE) {
            ps2Port1DataBuffer[ps2Port1DataLength] = data;
            ps2Port1DataLength++;
        }
        stdio::kprintf("PS/2 port1 %x\n", data);
    }
}

void ps2Port2IntHandler(registers_t* r) {
    uint8_t data;
    if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
        data = io::inb(IO_DATA);
        if (ps2Port2DataLength < PS2_PORTS_DATA_BUFFER_SIZE) {
            ps2Port2DataBuffer[ps2Port2DataLength] = data;
            ps2Port2DataLength++;
        }
        stdio::kprintf("PS/2 port2 %x\n", data);
    }
}

uint8_t ps2::install() {
    uint8_t data;
    bool isDualChannel; // true=PS/2 Is dual channel, false=Single chanel only

    // Determine if PS/2 Controller exists through ACPI Check bit 1 (value = 2, the "8042" flag) in the "IA PC Boot Architecture Flags" field at offset 109 in the Fixed ACPI Description Table (FADT). If this bit is clear, then there is no PS/2 Controller

    // Disable PS/2 ports to avoid PS/2 devices from sending data during Controller configuration
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_DISABLE);
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_DISABLE);

    isr::registerIsrHandler(IRQ1, ps2Port1IntHandler);   // Register PS/2 Controller port1 IRQ handler
    isr::registerIsrHandler(IRQ12, ps2Port2IntHandler);  // Register PS/2 Controller port2 IRQ handler

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
        data |= CTRL_CONF_PS2_PORT2_INT_ENABLED + CTRL_CONF_PS2_PORT1_INT_ENABLED;

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

    // Clear port1 buffer to be used to receive device reset response data
    ps2Port1DataLength = 0;

    // Reset PS/2 Device 1
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
    io::outb(IO_DATA, CTRL_CMD_PS2_RESET);

    // Check if port1 reset succeeded
    if (!memutils::memchr(ps2Port1DataBuffer, DEVICE_RESP_ACK, ps2Port1DataLength)) { // ACK FAILED
        return PS2_ERROR_PORT1_RESET_ACK_ERROR;
    } else if (!memutils::memchr(ps2Port1DataBuffer, DEVICE_RESP_SELF_TEST_SUCCESS, ps2Port1DataLength)) { // SELF TEST FAILED
        return PS2_ERROR_PORT1_RESET_SELF_TEST_ERROR;
    }

    // PS/2 Device reset
    if (isDualChannel) {
        // Clear port2 buffer to be used to receive device reset response data
        ps2Port2DataLength = 0;

        // Reset PS/2 Device 2
        // Check if input buffer data needs to be read
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_IN_WRITE);
        io::outb(IO_DATA, CTRL_CMD_PS2_RESET);

        // Check if port2 reset succeeded
        if (!memutils::memchr(ps2Port2DataBuffer, DEVICE_RESP_ACK, ps2Port2DataLength)) { // ACK FAILED
            return PS2_ERROR_PORT2_RESET_ACK_ERROR;
        } else if (!memutils::memchr(ps2Port1DataBuffer, DEVICE_RESP_SELF_TEST_SUCCESS, ps2Port1DataLength)) { // SELF TEST FAILED
            return PS2_ERROR_PORT2_RESET_SELF_TEST_ERROR;
        }
    }

    stdio::kprintf("PS/2 - port0 identify\n");

    // Clear port1 buffer to be used to receive device type data
    ps2Port1DataLength = 0;
    // Detect port device types
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
    io::outb(IO_DATA, DEVICE_CMD_DISABLE_SCANNING);
    if (memutils::memchr(ps2Port1DataBuffer, DEVICE_RESP_ACK, ps2Port1DataLength)) { // ACK SUCCESS
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
        io::outb(IO_DATA, DEVICE_CMD_IDENTIFY);
    }

    return PS2_NO_ERROR;
}

uint8_t ps2::testPort(uint8_t port) {
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