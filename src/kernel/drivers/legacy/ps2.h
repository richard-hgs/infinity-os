#pragma once
#ifndef _PS2_H_
#define _PS2_H_

// libc
#include <stdint.h>

/**
 * @brief PS/2 Error Codes
 * 
 */
#define PS2_NO_ERROR                                    0
#define PS2_ERROR_CONTROLLER_TEST                       1
#define PS2_ERROR_PORT1_TEST_CLOCK_LINE_STUCK_LOW       2
#define PS2_ERROR_PORT1_TEST_CLOCK_LINE_STUCK_HIGH      3
#define PS2_ERROR_PORT1_TEST_DATA_LINE_STUCK_LOW        4
#define PS2_ERROR_PORT1_TEST_DATA_LINE_STUCK_HIGH       5
#define PS2_ERROR_PORT2_TEST_CLOCK_LINE_STUCK_LOW       6
#define PS2_ERROR_PORT2_TEST_CLOCK_LINE_STUCK_HIGH      7
#define PS2_ERROR_PORT2_TEST_DATA_LINE_STUCK_LOW        8
#define PS2_ERROR_PORT2_TEST_DATA_LINE_STUCK_HIGH       9
#define PS2_ERROR_PORT1_RESET_ACK_ERROR                10
#define PS2_ERROR_PORT1_RESET_SELF_TEST_ERROR          11
#define PS2_ERROR_PORT2_RESET_ACK_ERROR                12
#define PS2_ERROR_PORT2_RESET_SELF_TEST_ERROR          13
#define PS2_ERROR_PORT1_DISABLE_SCANNING_ERROR         14
#define PS2_ERROR_STATUS_BIT_EQUALS_TIMEOUT            15
#define PS2_ERROR_NO_DEVICE_CONNECTED                  16
#define PS2_ERROR_PORT1_ENABLE_SCANNING_ERROR          17
#define PS2_ERROR_DEVICE_TYPE_NOT_CONNECTED            18

/**
 * @brief PS/2 Device to send/read data through serial com
 * 
 */
#define PS2_DEVICE_TYPE_KEYBOARD 0
#define PS2_DEVICE_TYPE_MOUSE    1

/**
 * @brief PS/2 Common device responses
 * 
 */
#define DEVICE_RESP_ERROR                           0x00    // Error or internal buffer overrun
#define DEVICE_RESP_ACK                             0xFA    // Command acknowledged (ACK)
#define DEVICE_RESP_SELF_TEST_SUCCESS               0xAA    // Self test passed (sent after "0xFF (reset)" command or keyboard power up)

/**
 * @brief PS/2 Size of the buffers of the response of controller ports
 * 
 */
#define PS2_PORTS_DATA_BUFFER_SIZE 10

/**
 * @brief Structure used to check if one byte of a buffer equals to required value if not return the given error.
 * 
 */
typedef struct BufferContains {
    uint8_t required;
    uint8_t error;
} BufferContains_t;

/**
 * @brief 8042 PS/2 Controller
 * 
 * DATA_PORT_0x60:
 *    - Is used for reading data that was received from a PS/2 device or from the PS/2 controller itself 
 *    and writing data to a PS/2 device or to the PS/2 controller itself.
 * 
 * STATUS_REGISTER_0x64:
 *    - The Status Register contains various flags that show the state of the PS/2 controller. The meanings for each bit are:
 *    ________________________________________________________________________________________________________________________________
 *   |      7       |       6       |    5     |     4    |       3        |      2      |           1         |          0           |
 *   | Parity Error | Timeout Error | Reserved | Reserved | Command / Data | System Flag | Input Buffer Status | Output Buffer Status |
 *    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *    - Bit 0: Output buffer status (0=Empty, 1=Full) (must be set before attempting to read data from IO port 0x60)
 *    - Bit 1: Input buffer status  (0=Empty, 1=Full) (must be clear before attempting to write data to IO port 0x60 or IO port 0x64)
 *    - Bit 2: System Flag Meant to be cleared on reset and set by firmware (via. PS/2 Controller Configuration Byte) if the system passes self tests (POST)
 *    - Bit 3: Command/data (0 = data written to input buffer is data for PS/2 device, 1 = data written to input buffer is data for PS/2 controller command)
 *    - Bit 4: Unknown (chipset specific) May be "keyboard lock" (more likely unused on modern systems)
 *    - Bit 5: Unknown (chipset specific) May be "receive time-out" or "second PS/2 port output buffer full"
 *    - Bit 6: Time-out error (0=No error, 1=Time-out error)
 *    - Bit 7: Parity error (0=No error, 1=Parity error)
 * 
 * COMMAND_REGISTER_0x64:
 *    - The Command Port (IO Port 0x64) is used for sending commands to the PS/2 Controller (not to PS/2 devices).
 *    - COMMANDS:
 *       _________________________________________________________________________________________________________________________________________________________________________________________________________________________________________________
 *      | COMMAND BYTE |             MEANING                                                                         |           RESPONSE BYTE                                                                                                            |
 *      |     0x20     | Read "byte 0" from internal RAM                                                             | Controller Configuration Byte (see below)                                                                                          |
 *      | 0x21 to 0x3F | Read "byte N" from internal RAM (where 'N' is the command byte & 0x1F)                      | Unknown (only the first byte of internal RAM has a standard purpose)                                                               |
 *      |     0x60     | Write next byte to "byte 0" of internal RAM (Controller Configuration Byte, see below)      | None                                                                                                                               |
 *      | 0x61 to 0x7F | Write next byte to "byte N" of internal RAM (where 'N' is the command byte & 0x1F)          | None                                                                                                                               |
 *      |     0xA7     | Disable second PS/2 port (only if 2 PS/2 ports supported)                                   | None                                                                                                                               |
 *      |     0xA8     | Enable second PS/2 port (only if 2 PS/2 ports supported)                                    | None                                                                                                                               |
 *      |     0xA9     | Test second PS/2 port (only if 2 PS/2 ports supported)                                      | (0x00 test passed) (0x01 clock line stuck low) (0x02 clock line stuck high) (0x03 data line stuck low) (0x04 data line stuck high) |
 *      |     0xAA     | Test PS/2 Controller                                                                        | (0x55 test passed) (0xFC test failed)                                                                                              |
 *      |     0xAB     | Test first PS/2 port                                                                        | (0x00 test passed) (0x01 clock line stuck low) (0x02 clock line stuck high) (0x03 data line stuck low) (0x04 data line stuck high) |
 *      |     0xAC     | Diagnostic dump (read all bytes of internal RAM)                                            | Unknown                                                                                                                            |
 *      |     0xAD     | Disable first PS/2 port                                                                     | None                                                                                                                               |
 *      |     0xAE     | Enable first PS/2 port                                                                      | None                                                                                                                               |
 *      |     0xC0     | Read controller input port                                                                  | Unknown (none of these bits have a standard/defined purpose)                                                                       |
 *      |     0xC1     | Copy bits 0 to 3 of input port to status bits 4 to 7                                        | None                                                                                                                               |
 *      |     0xC2     | Copy bits 4 to 7 of input port to status bits 4 to 7                                        | None                                                                                                                               |
 *      |     0xD0     | Read Controller Output Port                                                                 | Controller Output Port (see below)                                                                                                 |
 *      |     0xD1     | Write next byte to Controller Output Port (see below) Check if output buffer is empty first | None                                                                                                                               |
 *      |     0xD2     | Write next byte to first PS/2 port output buffer (only if 2 PS/2 ports supported)           | None                                                                                                                               |
 *      |              | (makes it look like the byte written was received from the first PS/2 port)                 |                                                                                                                                    |
 *      |     0xD3     | Write next byte to second PS/2 port output buffer (only if 2 PS/2 ports supported)          | None                                                                                                                               |
 *      |              | (makes it look like the byte written was received from the second PS/2 port)                |                                                                                                                                    |
 *      |     0xD4     | Write next byte to second PS/2 port input buffer (only if 2 PS/2 ports supported)           | None                                                                                                                               |
 *      |              | (sends next byte to the second PS/2 port)                                                   |                                                                                                                                    |
 *      | 0xF0 to 0xFF | Pulse output line low for 6 ms. Bits 0 to 3 are used as a mask                              | None                                                                                                                               |
 *      |              | (0 = pulse line, 1 = don't pulse line) and correspond to 4 different output lines.          |                                                                                                                                    |
 *      |              | Note: Bit 0 corresponds to the "reset" line. The other output lines don't have a            |                                                                                                                                    |
 *      |              | standard/defined purpose.                                                                   |                                                                                                                                    |
 *       ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 * 
 * PS/2 CONTROLLER CONFIGURATION BYTE:
 *    - Commands 0x20 and 0x60 let you read and write the PS/2 Controller Configuration Byte. 
 *      This configuration byte has the following format:
 *      _____________________________________________________________________________________________________________________________________________________________________________________
 *     |             7             |             6              |      5      |        4       |          3            |             2          |             1               |      0       |
 *     | First PS/2 port interrupt | Second PS/2 port interrupt | System Flag | Should be zero | First PS/2 port clock | Second PS/2 port clock | First PS/2 port translation | Must be zero |
 *      ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *    - Bit 0: First PS/2 port interrupt  (1=Enabled, 0=Disabled)
 *    - Bit 1: Second PS/2 port interrupt (1=Enabled, 0=Disabled, only if 2 PS/2 ports supported)
 *    - Bit 2: System Flag (1=System passed POST, 0=Your OS shouldn't be running)
 *    - Bit 3: Should be zero
 *    - Bit 4: First PS/2 port clock  (1=Disabled, 0=Enabled)
 *    - Bit 5: Second PS/2 port clock (1=Disabled, 0=Enabled, only if 2 PS/2 ports supported)
 *    - Bit 6: First PS/2 port translation (1=Enabled, 0=Disabled)
 *    - Bit 7: Must be zero
 * 
 * PS/2 CONTROLLER OUTPUT PORT:
 *    - Commands 0xD0 and 0xD1 let you read and write the PS/2 Controller Output Port. This output port has the following format:
 *      _______________________________________________________________________________________________________________________________________________________________________________________________________________
 *     |               7               |               6                |               5            |               4            |         3         |          2         |         1         |             0         |
 *     | First PS/2 port data (output) | First PS/2 port clock (output) | 2º PS/2 Output buffer full | 1º PS/2 Output buffer full | 2º PS/2 port data | 2º PS/2 port clock | A20 gate (output) | System reset (output) |
 *      ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 */
namespace ps2 {
    /**
     * @brief Initialize and configure the PS/2 Controller with PIC (IRQ1=Port1, IRQ12=Port2) located at isr.cpp
     * 
     */
    uint8_t install();

    /**
     * @brief Self test given PS/2 Controller port
     * 
     * @param port      Port to test (0xAB=CTRL_CMD_PS2_CTRL_PORT1_TEST or 0xA9=CTRL_CMD_PS2_CTRL_PORT2_TEST)
     * @return uint8_t  Result error of the test:
     *                  - PS2_ERROR_PORT1_TEST_CLOCK_LINE_STUCK_LOW       2
     *                  - PS2_ERROR_PORT1_TEST_CLOCK_LINE_STUCK_HIGH      3
     *                  - PS2_ERROR_PORT1_TEST_DATA_LINE_STUCK_LOW        4
     *                  - PS2_ERROR_PORT1_TEST_DATA_LINE_STUCK_HIGH       5
     *                  - PS2_ERROR_PORT2_TEST_CLOCK_LINE_STUCK_LOW       6
     *                  - PS2_ERROR_PORT2_TEST_CLOCK_LINE_STUCK_HIGH      7
     *                  - PS2_ERROR_PORT2_TEST_DATA_LINE_STUCK_LOW        8
     *                  - PS2_ERROR_PORT2_TEST_DATA_LINE_STUCK_HIGH       9
     */
    uint8_t testPort(uint8_t port);

    /**
     * @brief Read data on the given PS/2 Controller port of the specified device
     * 
     * @param data Ptr where to store the data received. 0=Error or device not found, or the Data received    
     */
    void readData(uint8_t* data);


    /**
     * @brief Send a data to one of the devices if device is present and is configured successfully.
     * 
     * @param deviceType             IN  - One of (0=PS2_DEVICE_TYPE_KEYBOARD, 1=PS2_DEVICE_TYPE_MOUSE)
     * @param data                   IN  - Data to send to the given device
     * @param allValues              IN  - Response in asc order that the device should contains. This is not a equality check, is a contains check.
     *                               Since the buffer can contains some data we don't want.
     * @param allValuesLen           IN  - NULL or The length of the list of contains values to check in buffer response.
     * @param respData               OUT - Buffer containing the response of this command after "values contains match".
     * @param respLen                OUT - Size of the resp data received.
     * @return uint8_t                0 = PS2_NO_ERROR(Operation Suceeded)     -> Operation success
     *                               15 = PS2_ERROR_STATUS_BIT_EQUALS_TIMEOUT  -> Input bit status not equals to 0, so we cant send the command.
     *                               18 = PS2_ERROR_DEVICE_TYPE_NOT_CONNECTED  -> Device not connected or some error happened inside the ps2::install() function.
     *                               XX = Or one of the errors in BufferContains values if provided.
     */
    uint8_t sendDataToDevice(uint8_t deviceType, uint8_t data, 
                             BufferContains_t* allValues, uint8_t allValuesLen, 
                             BufferContains_t* oneOfValues, uint8_t oneOfValuesLen, 
                             uint8_t* oneOfValueFound,
                             unsigned char** respData, uint8_t* respLen);
}

#endif