// libc 
#include <stdbool.h>
// stdlibs
#include "bitwise.h"
#include "stdlib.h"
// cpu
#include "isr.h"
// stdio
#include "stdio.h" // Debug
// memory
#include "memutils.h"
// sys
#include "io.h"
// drivers
#include "pit.h"
#include "keyboard.h"
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

#define DEVICE_CMD_ENABLE_SCANNING                  0xF4    // Enable scanning  (If Keyboard=will send scan codes, If Mouse=Enable Data Reporting)
#define DEVICE_CMD_DISABLE_SCANNING                 0xF5    // Disable scanning (If Keyboard=won't send scan codes, If Mouse=Disable Data Reporting)
#define DEVICE_CMD_IDENTIFY                         0xF2    // Identify Device Type

/**
 * @brief PS/2 Device types
 * 
 */
// None=Ancient AT keyboard with translation enabled in the PS/Controller (not possible for the second PS/2 port)
#define DEVICE_ID_STANDARD_MOUSE                  0x00    // Standard PS/2 mouse
#define DEVICE_ID_MOUSE_WITH_SCROLL_WHEEL         0x03    // Mouse with scroll wheel
#define DEVICE_ID_5_BUTTON_MOUSE                  0x04    // 5-button mouse
#define DEVICE_ID_MF2_KEYBOARD_1_1                0xAB    // ‾|--> MF2 keyboard with translation enabled in the   
#define DEVICE_ID_MF2_KEYBOARD_1_2                0x41    // _|    PS/Controller (not possible for the second PS/2 port)
#define DEVICE_ID_MF2_KEYBOARD_2_1                0xAB    // ‾|--> MF2 keyboard with translation enabled in the   
#define DEVICE_ID_MF2_KEYBOARD_2_2                0xC1    // _|    PS/Controller (not possible for the second PS/2 port)
#define DEVICE_ID_MF2_KEYBOARD_3_1                0xAB    // ‾|--> MF2 keyboard  
#define DEVICE_ID_MF2_KEYBOARD_3_2                0x83    // _|

/**
 * @brief PS/2 Device ids translated to be simple inside the kernel
 * 
 */
#define PS2_DEVICE_ID_KEYBOARD_ANCIENT_AT         0x00
#define PS2_DEVICE_ID_KEYBOARD_1                  0x01
#define PS2_DEVICE_ID_KEYBOARD_2                  0x02
#define PS2_DEVICE_ID_KEYBOARD_3                  0x03
#define PS2_DEVICE_ID_MOUSE_STANDARD              0x04
#define PS2_DEVICE_ID_MOUSE_WITH_SCROLL_WHEEL     0x05
#define PS2_DEVICE_ID_MOUSE_WITH_5_BUTTON         0x06

#define TIMEOUT_COUNT 1000 // Max loop count before timeout reached
#define TIMEOUT_SLEEP 1    // Millis Time to wait for each loop count before try again

unsigned char ps2Port1DataBuffer[PS2_PORTS_DATA_BUFFER_SIZE];
unsigned char ps2Port2DataBuffer[PS2_PORTS_DATA_BUFFER_SIZE];

uint8_t ps2Port1DataLength = 0;
uint8_t ps2Port2DataLength = 0;

bool isDualChannel; // true=PS/2 Is dual channel, false=Single chanel only
bool isPort1DevicePresent; // true=Device is connected, false=Device not connected
bool isPort2DevicePresent; // true=Device is connected, false=Device not connected

uint8_t ps2Port1DeviceId; // PS/2 Port 1 device id PS2_DEVICE_ID_...
uint8_t ps2Port2DeviceId; // PS/2 Port 2 device id PS2_DEVICE_ID_...

uint8_t ps2Port1DeviceType; // PS/2 Port 1 device type PS2_DEVICE_TYPE_...
uint8_t ps2Port2DeviceType; // PS/2 Port 2 device type PS2_DEVICE_TYPE_...

// uint8_t ps2KeybordDevicePort; // PS/2 Port of the keyboard device
// uint8_t ps2MouseDevicePort;   // PS/2 Port of the mouse device

void ps2Port1IntHandler(registers_t* r) {
    uint8_t data;
    if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
        data = io::inb(IO_DATA);
        if (ps2Port1DataLength < PS2_PORTS_DATA_BUFFER_SIZE) {
            ps2Port1DataBuffer[ps2Port1DataLength] = data;
            ps2Port1DataLength++;
        }
        // stdio::kprintf("PS/2 port1 %x\n", data);
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
        // stdio::kprintf("PS/2 port2 %x\n", data);
    }
}

/**
 * @brief Wait until buffer contains specified allValues in the same order
 * 
 * @param port            IN - 1=ps2Port1DataBuffer, 2=ps2Port2DataBuffer
 * @param allValues       IN - Required values to find in search buffer in the same order
 * @param allValuesLen    IN - Size of the allValues buffer
 * @param oneOfValues     IN - Required only one value of the list in search buffer in any order
 * @param oneOfValuesLen  IN - Size of the oneOfValues buffer
 * @param oneOfFound      OUT - If oneOfValues found return the value found.
 * @return int            0=SUCCESS, or ERROR_CODE otherwise
 */
uint8_t waitUntilBufferContains(
    uint8_t port, 
    BufferContains_t* allValues, 
    uint8_t allValuesLen, 
    BufferContains_t* oneOfValues, 
    uint8_t oneOfValuesLen,
    uint8_t* oneOfFound
) {
    uint8_t i;
    uint8_t lastFoundIndex = 0;
    uint32_t countdownTimeout = TIMEOUT_COUNT;
    if (allValuesLen > 0 && allValues != NULL) { // Check if device will respond the specified sequence of bytes
        while(countdownTimeout > 0 && lastFoundIndex < allValuesLen) {
            countdownTimeout--;

            // Check if buffer contains all the required values in the same order
            for(i=lastFoundIndex; i<(lastFoundIndex + 1 <= allValuesLen ? lastFoundIndex + 1 : allValuesLen); i++) {
                const void* ptr = port == 1 ? ps2Port1DataBuffer : ps2Port2DataBuffer;
                int valueAt = allValues[i].required;
                uint32_t bufferLen = port == 1 ? ps2Port1DataLength : ps2Port2DataLength;
                if (memutils::memchr(ptr, valueAt, bufferLen)) {
                    lastFoundIndex++;
                }
            }
            pit::ksleep(TIMEOUT_SLEEP);
        }
    }

    if (oneOfValuesLen > 0 && oneOfValues != NULL && lastFoundIndex == allValuesLen) {
        while (countdownTimeout > 0 && lastFoundIndex < allValuesLen + 1) {
            countdownTimeout--;
            // Check if buffer contains one of the required values in the same order
            for(i=0; i<oneOfValuesLen; i++) {
                const void* ptr = port == 1 ? ps2Port1DataBuffer : ps2Port2DataBuffer;
                int valueAt = oneOfValues[i].required;
                uint32_t bufferLen = port == 1 ? ps2Port1DataLength : ps2Port2DataLength;
                if (memutils::memchr(ptr, valueAt, bufferLen)) {
                    lastFoundIndex++;
                    if (oneOfFound != NULL) {
                        *oneOfFound = valueAt;
                    }
                    break;
                }
            }
            pit::ksleep(TIMEOUT_SLEEP);
        }
    }

    if (
        (
            (allValuesLen == 0 || allValues == NULL) &&                                       // --| No validation required always succeded.
            (oneOfValuesLen == 0 || oneOfValues == NULL)                                      // --|
        ) || 
        ((oneOfValuesLen == 0 || oneOfValues == NULL) && lastFoundIndex == allValuesLen) ||   // If only the all values informed and succeeded.
        (lastFoundIndex == allValuesLen + 1)                                                  // If last or both validations succeeded.
    ) {
        return PS2_NO_ERROR;
    } else if (allValuesLen > 0 && lastFoundIndex < allValuesLen) { // If only the all values informed one value not found.
        return allValues[lastFoundIndex].error;
    } else if (oneOfValuesLen > 0 && lastFoundIndex == allValuesLen) { // No value found for one of values.
        return oneOfValues[0].error;
    } else { // Unexpected behavior, should never gets here
        return PS2_NO_ERROR;
    }
}

uint8_t waitUntilStatusBitEquals(uint8_t bitOffset, uint8_t bitValue) {
    uint32_t countdownTimeout = 2147483646; // Max Int
    while(countdownTimeout > 0 && ((io::inb(IO_CR_STATUS) >> bitOffset) & 0x1) != (bitValue & 0x1)) { 
        // Until bit offset of the status register equals to bitValue continue loop until countdown reaches 0
        countdownTimeout--;
    }

    return countdownTimeout > 0 ? PS2_NO_ERROR : PS2_ERROR_STATUS_BIT_EQUALS_TIMEOUT;
}

uint8_t waitDeviceIdentifierResponse(uint8_t port) {
    uint32_t countdownTimeout = TIMEOUT_COUNT;
    uint8_t ackIndex = 255; // 255 is a placeholder for NO INDEX FOUND. The port data buffer length must be less than this value.
    uint8_t i;
    while(countdownTimeout > 0) {
        countdownTimeout--;
        unsigned char* ptr = port == 1 ? ps2Port1DataBuffer : ps2Port2DataBuffer;
        uint32_t bufferLen = port == 1 ? ps2Port1DataLength : ps2Port2DataLength;
        if (bufferLen > 0) {
            if (ackIndex == 255) { // 255 is a placeholder for NO INDEX FOUND. The port data buffer length must be less than this value.
                for (i=0; i<bufferLen; i++) { // Search for ack index
                    if (ptr[i] == DEVICE_RESP_ACK) {
                        ackIndex = i;
                        break;
                    }
                }
            }

            if (ackIndex != 255 && ptr[ackIndex] == DEVICE_RESP_ACK) {
                // Command aknowledge by the keyboard
                if (bufferLen > 2) {
                    // Keyboard
                    if (
                        ptr[ackIndex + 1] == DEVICE_ID_MF2_KEYBOARD_1_1 &&
                        ptr[ackIndex + 2] == DEVICE_ID_MF2_KEYBOARD_1_2
                    ) {
                        return PS2_DEVICE_ID_KEYBOARD_1;
                    } else if (
                        ptr[ackIndex + 1] == DEVICE_ID_MF2_KEYBOARD_2_1 &&
                        ptr[ackIndex + 2] == DEVICE_ID_MF2_KEYBOARD_2_2
                    ) {
                        return PS2_DEVICE_ID_KEYBOARD_2;
                    } else if (
                        ptr[ackIndex + 1] == DEVICE_ID_MF2_KEYBOARD_3_1 &&
                        ptr[ackIndex + 2] == DEVICE_ID_MF2_KEYBOARD_3_2
                    ) {
                        return PS2_DEVICE_ID_KEYBOARD_3;
                    }
                } else if (bufferLen > 1) {
                    if (ptr[ackIndex + 1] == DEVICE_ID_STANDARD_MOUSE) {
                        return PS2_DEVICE_ID_MOUSE_STANDARD;
                    } else if (ptr[ackIndex + 1] == DEVICE_ID_MOUSE_WITH_SCROLL_WHEEL) {
                        return PS2_DEVICE_ID_MOUSE_WITH_SCROLL_WHEEL;
                    } else if (ptr[ackIndex + 1] == DEVICE_ID_5_BUTTON_MOUSE) {
                        return PS2_DEVICE_ID_MOUSE_WITH_5_BUTTON;
                    }
                }
            }
        }
        pit::ksleep(TIMEOUT_SLEEP);
    }

    return PS2_DEVICE_ID_KEYBOARD_ANCIENT_AT;
}

void registerDriverIrqForDeviceType(uint8_t deviceType) {
    // Pass the control to the interrupt driver
    if (ps2Port1DeviceType == deviceType) {
        if (deviceType == PS2_DEVICE_TYPE_KEYBOARD) {
            isr::registerIsrHandler(IRQ1, kbd::keyboardIntHandler);   // Register PS/2 Controller port1 IRQ handler
        } // else if (deviceType == PS2_DEVICE_TYPE_MOUSE) {
            // isr::registerIsrHandler(IRQ1, mouse::mouseIntHandler); // Register PS/2 Controller port2 IRQ handler
        // }
    } // else { same as above for IRQ12 }
}

uint8_t ps2::install() {
    BufferContains_t bufferContains[3];
    uint8_t data;
    uint8_t data2;

    // Determine if PS/2 Controller exists through ACPI Check bit 1 (value = 2, the "8042" flag) in the "IA PC Boot Architecture Flags" field at offset 109 in the Fixed ACPI Description Table (FADT). If this bit is clear, then there is no PS/2 Controller

    // Disable PS/2 port 1 to avoid PS/2 devices from sending data during Controller configuration
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_DISABLE);

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }


    
    // Disable PS/2 port 2 to avoid PS/2 devices from sending data during Controller configuration
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_DISABLE);

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }

    isr::registerIsrHandler(IRQ1, ps2Port1IntHandler);   // Register PS/2 Controller port1 IRQ handler
    isr::registerIsrHandler(IRQ12, ps2Port2IntHandler);  // Register PS/2 Controller port2 IRQ handler

    // Flush Controller Output Buffer to avoid data stuck in buffer
    io::inb(IO_DATA);                 // Read/Discard data from buffer

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }

    // Change PS/2 Controller configuration
    io::outb(IO_CR_STATUS, CTRL_CMD_READ_CONFIGURATION);    // Request to controller to send controller configuration to data buffer


    // Wait until Output bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_OUT_BUFFER_STATUS), 1);
    if (data != PS2_NO_ERROR) {
        // Output status bit not cleared
        return data;
    }

    // Read configuration sent by controller to data buffer
    data = io::inb(IO_DATA);

    // stdio::kprintf("PS/2 - old config: %08b\n", data);

    // Enable first PS/2 port interrupt bit 0
    data |= CTRL_CONF_PS2_PORT2_INT_ENABLED + CTRL_CONF_PS2_PORT1_INT_ENABLED;

    // Wait until Input bit status cleared
    data2 = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data2 != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data2;
    }

    // Cmd to Write configuration
    io::outb(IO_CR_STATUS, CTRL_CMD_WRITE_CONFIGURATION);
    // Wait until Input bit status cleared
    data2 = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data2 != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data2;
    }
    // Write configuration
    io::outb(IO_DATA, data);

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }

    // Read PS/2 Controller configuration to check the new configuration
    io::outb(IO_CR_STATUS, CTRL_CMD_READ_CONFIGURATION);    // Request to controller to send controller configuration to data buffer
    
    // Wait until Output bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_OUT_BUFFER_STATUS), 1);
    if (data != PS2_NO_ERROR) {
        // Output status bit not cleared
        return data;
    }
    
    // Read configuration sent by controller to data buffer
    data = io::inb(IO_DATA);
    // stdio::kprintf("PS/2 - new config: %08b\n", data);
    
    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }

    // PS/2 Controller self test
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_CTRL_TEST);

    // Wait until Output bit status set
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_OUT_BUFFER_STATUS), 1);
    if (data != PS2_NO_ERROR) {
        // Output status bit not set
        return data;
    }

    data = io::inb(IO_DATA);
    if (data == CTRL_TEST_RESULT_FAILED) { // Controller self test failed
        return PS2_ERROR_CONTROLLER_TEST;
    }

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }

    // Check if PS/2 is dual channel by enabling port 2
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_ENABLE);      // Enalbe PS/2 Second port controller

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }

    io::outb(IO_CR_STATUS, CTRL_CMD_READ_CONFIGURATION);    // Request to controller to send controller configuration to data buffer
    
    // Wait until Output bit status set
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_OUT_BUFFER_STATUS), 1);
    if (data != PS2_NO_ERROR) {
        // Output status bit not set
        return data;
    }
    
    // Read configuration sent by controller to data buffer
    data = io::inb(IO_DATA);

    // stdio::kprintf("PS/2 - config: %08b\n", data);

    if (!(data & CTRL_CONF_PS2_PORT2_CLOCK_ENABLED)) { // If this bit is clear it's a dual channel, if set isn't dual channel
        // Is dual channel
        isDualChannel = true;
        // stdio::kprintf("PS/2 - Is dual channel\n", data);
    }
    
    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
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

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }

    // Enable all PS/2 ports that exists
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_ENABLE);

    if (isDualChannel) {
        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_ENABLE);
    }

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }

    // Clear port1 buffer to be used to receive device reset response data
    ps2Port1DataLength = 0;

    // Reset PS/2 Device 1
    io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data != PS2_NO_ERROR) {
        // Input status bit not cleared
        return data;
    }
    io::outb(IO_DATA, CTRL_CMD_PS2_RESET);

    // Check if port1 reset succeeded
    bufferContains[0] = {DEVICE_RESP_ACK, PS2_ERROR_PORT1_RESET_ACK_ERROR};
    bufferContains[1] = {DEVICE_RESP_SELF_TEST_SUCCESS, PS2_ERROR_PORT1_RESET_SELF_TEST_ERROR};
    data = waitUntilBufferContains(1, bufferContains, 2, NULL, 0, NULL);
    if (data == PS2_NO_ERROR) {
        // Reset succeeded and device returned response with success
        isPort1DevicePresent = true;
    }

    // PS/2 Device reset
    if (isDualChannel) {
        // Clear port2 buffer to be used to receive device reset response data
        ps2Port2DataLength = 0;

        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }

        // Reset PS/2 Device 2
        // Check if input buffer data needs to be read
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT2_IN_WRITE);
        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }
        io::outb(IO_DATA, CTRL_CMD_PS2_RESET);

        // Check if port2 reset succeeded
        bufferContains[0] = {DEVICE_RESP_ACK, PS2_ERROR_PORT2_RESET_ACK_ERROR};
        bufferContains[1] = {DEVICE_RESP_SELF_TEST_SUCCESS, PS2_ERROR_PORT2_RESET_SELF_TEST_ERROR};
        data = waitUntilBufferContains(2, bufferContains, 2, NULL, 0, NULL);
        if (data == PS2_NO_ERROR) {
            // Reset succeeded and device returned response with success
            isPort2DevicePresent = true;
        }
    }

    if (!isPort1DevicePresent && !isPort2DevicePresent) {
        return PS2_ERROR_NO_DEVICE_CONNECTED;
    }

    if (isPort1DevicePresent) {
        // Clear port1 buffer to be used to receive device type data
        ps2Port1DataLength = 0;

        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }

        // Detect port device types
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }
        io::outb(IO_DATA, DEVICE_CMD_DISABLE_SCANNING);

        // Check if port1 Disable scanning succeeded
        bufferContains[0] = {DEVICE_RESP_ACK, PS2_ERROR_PORT1_DISABLE_SCANNING_ERROR};
        data = waitUntilBufferContains(1, bufferContains, 1, NULL, 0, NULL);
        if (data != PS2_NO_ERROR) {
            // Timeout error. Device doesn't returned the required data.
            return data;
        }

        // Clear port1 buffer to be used to receive device type data
        ps2Port1DataLength = 0;

        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }

        // Request device in port 1 to identify yourself
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }
        io::outb(IO_DATA, DEVICE_CMD_IDENTIFY);

        ps2Port1DeviceId = waitDeviceIdentifierResponse(1);

        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }
        
        // Enable device scanning codes or report
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
        // Wait until Input bit status cleared
        data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
        if (data != PS2_NO_ERROR) {
            // Input status bit not cleared
            return data;
        }

        // Clear port1 buffer to be used to receive device enable scanning response
        ps2Port1DataLength = 0;

        io::outb(IO_DATA, DEVICE_CMD_ENABLE_SCANNING);

        // Wait until buffer contains
        bufferContains[0] = {DEVICE_RESP_ACK, PS2_ERROR_PORT1_ENABLE_SCANNING_ERROR};
        data = waitUntilBufferContains(1, bufferContains, 1, NULL, 0, NULL);
        if (data != PS2_NO_ERROR) {
            // Timeout error. Device doesn't returned the required data.
            return data;
        }
        
        if (
            ps2Port1DeviceId == PS2_DEVICE_ID_KEYBOARD_ANCIENT_AT ||
            ps2Port1DeviceId == PS2_DEVICE_ID_KEYBOARD_1 ||
            ps2Port1DeviceId == PS2_DEVICE_ID_KEYBOARD_2 ||
            ps2Port1DeviceId == PS2_DEVICE_ID_KEYBOARD_3
        ) { 
            // Port1 is the keyboard device.
            // Pass the control to keyboard driver.
            // Register keyboard int handler in port 1 to handle keyboard interruptions.
            // Call keyboard driver install function.
            ps2Port1DeviceType = PS2_DEVICE_TYPE_KEYBOARD;
            isr::registerIsrHandler(IRQ1, kbd::keyboardIntHandler);   // Register PS/2 Controller port1 IRQ handler
            kbd::install();
        }

        // stdio::kprintf("PS/2 port1 device type: %d\n", ps2Port1DeviceId);
    }

    // if (isPort2DevicePresent) {
    //     // stdio::kprintf("port2 device present\n");
    // }

    return PS2_NO_ERROR;
}

uint8_t ps2::testPort(uint8_t port) {
    uint8_t data;

    // Wait until Input bit status cleared
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (data) {
        // Input status bit not cleared
        return data;
    }

    // Send the test command to the port
    io::outb(IO_CR_STATUS, port);

    // Wait until Output bit status set
    data = waitUntilStatusBitEquals(first_bit_set_index(STATUS_OUT_BUFFER_STATUS), 1);
    if (data) {
        // Output status bit not set
        return data;
    }
    
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

    return PS2_NO_ERROR;
}

void ps2::readData(uint8_t* data) {
    if (io::inb(IO_CR_STATUS) & STATUS_OUT_BUFFER_STATUS) { // Output buffer status Bit is set means that data buffer is full and we can read it now.
        *data = io::inb(IO_DATA);
    } else {
        *data = 0; // Error
    }
}

uint8_t ps2::sendDataToDevice(
    uint8_t deviceType, 
    uint8_t data, 
    BufferContains_t* allValues, 
    uint8_t allValuesLen, 
    BufferContains_t* oneOfValues, 
    uint8_t oneOfValuesLen, 
    uint8_t* oneOfValueFound,
    unsigned char** respData, 
    uint8_t* respLen
) {
    uint8_t mData;
    uint8_t portId;
    // Wait until Input bit status cleared
    mData = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (mData != PS2_NO_ERROR) {
        // Input status bit not cleared
        return mData;
    }

    if (ps2Port1DeviceType == deviceType) {
        if (!isPort1DevicePresent) {
            // Device is not present return error
            return PS2_ERROR_DEVICE_TYPE_NOT_CONNECTED;
        }

        // Clear port buffer to be used to receive device response
        ps2Port1DataLength = 0;
        portId = 1;

        // Register PS/2 Controller port1 IRQ handler
        isr::registerIsrHandler(IRQ1, ps2Port1IntHandler);

        // Tells the PS/2 controller that the next data goes to device in port1
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
    } else if (ps2Port2DeviceType == deviceType) {
        if (!isPort2DevicePresent) {
            // Device is not present return error
            return PS2_ERROR_DEVICE_TYPE_NOT_CONNECTED;
        }

        // Clear port buffer to be used to receive device response
        ps2Port2DataLength = 0;
        portId = 2;

        // Register PS/2 Controller port2 IRQ handler
        isr::registerIsrHandler(IRQ12, ps2Port2IntHandler);  

        // Tells the PS/2 controller that the next data goes to device in port2
        io::outb(IO_CR_STATUS, CTRL_CMD_PS2_PORT1_IN_WRITE);
    }

    // Wait until Input bit status cleared
    mData = waitUntilStatusBitEquals(first_bit_set_index(STATUS_IN_BUFFER_STATUS), 0);
    if (mData != PS2_NO_ERROR) { // Input status bit not cleared
        
        // Pass the control to the driver interrupt driver again
        registerDriverIrqForDeviceType(deviceType);

        return mData;
    }

    // Send data to device
    io::outb(IO_DATA, data);

    // Get device response
    mData = waitUntilBufferContains(portId, allValues, allValuesLen, oneOfValues, oneOfValuesLen, oneOfValueFound);

    // Pass the control to the driver interrupt driver again
    registerDriverIrqForDeviceType(deviceType);

    if (respData != NULL && respLen != NULL) {
        if (portId == 1) {
            *respData = ps2Port1DataBuffer;
            *respLen = ps2Port1DataLength;
        } else {
            *respData = ps2Port2DataBuffer;
            *respLen = ps2Port2DataLength;
        }
    }

    return mData;
}