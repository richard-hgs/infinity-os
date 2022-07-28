#pragma once
#ifndef _PIT_H_
#define _PIT_H_

// libc
#include <stdint.h>

/**
 * @brief PIT - Programmable Interval Timer
 * 
 * CHANNEL_0:
 *     - Connected directly to IRQ0. So it is best to use it only for purposes that should generate interrupts.
 * 
 * CHANNEL_1:
 *     - Channel 1 is unusable, and may not even exist.
 *     - In OLD days channel 1 was used to refresh DRAM wich was composed with tiny capacitors that needed to get voltage to keep data stored, 
 *       since capacitors lose their current over time until reach 0.
 * 
 * CHANNEL_2: 
 *     - Channel 2 is connected to the PC speaker, but can be used for other purposes without producing audible speaker tones.
 * 
 * ACCURACY:
 *     - Is typically accurate to within +/- 1.73 seconds per day.
 * 
 * IO_PORTS:
 *     - Each 8 bit data port is the same, and is used to set the counter's 16 bit reload value or read the channel's 16 bit current count.
 *     _____________________________________________________________________
 *    | I/O port  |   Usage                                                 |
 *    |   0x40    |   Channel 0 data port (read/write)                      |
 *    |   0x41    |   Channel 1 data port (read/write)                      |
 *    |   0x42    |   Channel 2 data port (read/write)                      |
 *    |   0x43    |   Mode/Command register (write only, a read is ignored) |
 *     ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾    
 * 
 * MODE/COMMAND_REGISTER:
 *     _________________________________________________________________________
 *    |  Bits   |     Usage                                                     |
 *    | 6 and 7 |     Select channel :                                          |
 *    |         |        0 0 = Channel 0                                        |
 *    |         |        0 1 = Channel 1                                        |
 *    |         |        1 0 = Channel 2                                        |
 *    |         |        1 1 = Read-back command (8254 only)                    |
 *    | 4 and 5 |     Access mode :                                             |
 *    |         |        0 0 = Latch count value command                        |
 *    |         |        0 1 = Access mode: lobyte only                         |
 *    |         |        1 0 = Access mode: hibyte only                         |
 *    |         |        1 1 = Access mode: lobyte/hibyte                       |
 *    | 1 to 3  |     Operating mode :                                          |
 *    |         |        0 0 0 = Mode 0 (interrupt on terminal count)           |
 *    |         |        0 0 1 = Mode 1 (hardware re-triggerable one-shot)      |
 *    |         |        0 1 0 = Mode 2 (rate generator)                        |
 *    |         |        0 1 1 = Mode 3 (square wave generator)                 |
 *    |         |        1 0 0 = Mode 4 (software triggered strobe)             |
 *    |         |        1 0 1 = Mode 5 (hardware triggered strobe)             |
 *    |         |        1 1 0 = Mode 2 (rate generator, same as 010b)          |
 *    |         |        1 1 1 = Mode 3 (square wave generator, same as 011b)   |
 *    | 0       |     BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD    |
 *     ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *     - Select Channel: 
 *         - Select which channel is being configured. 
 *         - Must always be valid on every write of the mode/command register, regardless of the other bits or the type of operation being performed.
 *     
 *     - Read Back:
 *         - (both bits set) is not supported on the old 8253 chips but should be supported on all AT and later computers except for PS/2.
 *     
 *     - Access Mode:
 *         - Tell the PIT what access mode you wish to use for the selected channel, and also specify the "counter latch" command to the CTC.
 *         - For the "read back" command, these bits have a different meaning.
 *         - For the remaining combinations, these bits specify what order data will be read and written to the data port for the associated PIT channel.
 * 
 *     - Operating Mode:
 *         - Initial Output State:
 *             - Every time the mode/command register is written to, all internal logic in the selected PIT channel is reset, and the output immediately 
 *               goes to its initial state (which depends on the mode).
 *         - Changing Reload Value:
 *             - A new reload value can be written to a PIT channel's data port at any time. The operating mode determines the exact effect that this will have.
 *         - Current Counter:
 *             - The current counter value is always either decremented or reset to the reload value on the falling edge of the (1.193182 MHz) input signal.
 *         - Current Counter Reload:
 *             - In modes where the current count is decremented when it is reloaded, the current count is not decremented on the same input clock pulse as the reload 
 *               it starts decrementing on the next input clock pulse.
 *         
 *         - Mode 0 – Interrupt On Terminal Count:
 *             - For this mode, when the mode/command register is written the output signal goes low and the PIT waits for the reload register to be set by software, 
 *               to begin the countdown.
 *             - After the reload register has been set, the current count will be set to the reload value on the next falling edge of the (1.193182 MHz) input signal.
 *             - Subsequent falling edges of the input signal will decrement the current count (if the gate input is high on the preceding rising edge of the input signal).
 *             - When the current count decrements from one to zero, the output goes high and remains high until another mode/command register is written or the reload 
 *               register is set again.
 *             - The current count will wrap around to 0xFFFF (or 0x9999 in BCD mode) and continue to decrement until the mode/command register or the reload register 
 *               are set, however this will not affect the output pin state.
 *             - The reload value can be changed at any time. In "lobyte/hibyte" access mode counting will stop when the first byte of the reload value is set.
 *             - Once the full reload value is set (in any access mode), the next falling edge of the (1.193182 MHz) input signal will cause the new reload value to 
 *               be copied into the current count, and the countdown will continue from the new value.
 *             - Generates interrupts on channel 0.
 * 
 *         - Mode 1 – Hardware Re-triggerable One-shot:
 *             - This mode is similar to mode 0 above, however counting doesn't start until a rising edge of the gate input is detected. For this reason it is not 
 *               usable for PIT channels 0 or 1 (where the gate input can't be changed).
 *             - When the mode/command register is written the output signal goes high and the PIT waits for the reload register to be set by software.
 *             - After the reload register has been set the PIT will wait for the next rising edge of the gate input.
 *             - Once this occurs, the output signal will go low and the current count will be set to the reload value on the next falling edge of the (1.193182 MHz) 
 *               input signal.
 *             - Subsequent falling edges of the input signal will decrement the current count.
 *             - When the current count decrements from one to zero, the output goes high and remains high until another mode/command register is written or the 
 *               reload register is set again.
 *             - The current count will wrap around to 0xFFFF (or 0x9999 in BCD mode) and continue to decrement until the mode/command register or the reload 
 *               register are set, however this will not affect the output pin state.
 *             - If the gate input signal goes low during this process it will have no effect.
 *             - However, if the gate input goes high again it will cause the current count to be reloaded from the reload register on the next falling edge 
 *               of the input signal, and restart the count again (the same as when counting first started).
 *             - The reload value can be changed at any time, however the new value will not affect the current count until the current count is reloaded 
 *               (on the next rising edge of the gate input). So if you want to do this, clear and then reset bit 0 of IO port 0x61, after modifying the reload value.
 * 
 *         - Mode 2 – Rate Generator:
 *             - This mode operates as a frequency divider.
 *             - When the mode/command register is written the output signal goes high and the PIT waits for the reload register to be set by software.
 *             - After the reload register has been set, the current count will be set to the reload value on the next falling edge of the (1.193182 MHz) input signal.
 *             - Subsequent falling edges of the input signal will decrement the current count (if the gate input is high on the preceding rising edge of the input signal).
 *             - When the current count decrements from two to one, the output goes low, and on the next falling edge of the (1.193182 MHz) input signal it will go high 
 *               again and the current count will be set to the reload value and counting will continue.
 *             - If the gate input goes low, counting stops and the output goes high immediately. Once the gate input has returned high, the next falling edge on input 
 *               signal will cause the current count to be set to the reload value and operation will continue.
 *             - The reload value can be changed at any time, however the new value will not affect the current count until the current count is reloaded 
 *               (when it is decreased from two to one, or the gate input going low then high). When this occurs counting will continue using the new reload value.
 *             - A reload value (or divisor) of one must not be used with this mode.
 *             - This mode creates a high output signal that drops low for one input signal cycle (0.8381 uS) which is too fast to make a difference to the PC speaker.
 *             - For this reason mode 2 is useless for producing sounds with PIT channel 2.
 *             - Typically, OSes and BIOSes use mode 3 (see below) for PIT channel 0 to generate IRQ 0 timer ticks, but some use mode 2 instead, to gain frequency 
 *               accuracy (frequency = 1193182 / reload_value Hz).
 * 
 *         - Mode 3 – Square Wave Generator:
 *             - For mode 3, the PIT channel operates as a frequency divider like mode 2, however the output signal is fed into an internal "flip flop" 
 *               to produce a square wave (rather than a short pulse).
 *             - The flip flop changes its output state each time its input state (or the output of the PIT channel's frequency divider) changes.
 *             - This causes the actual output to change state half as often, so to compensate for this the current count is decremented twice on each falling edge 
 *               of the input signal (instead of once), and the current count is set to the reload value twice as often.
 *             - When the mode/command register is written the output signal goes high and the PIT waits for the reload register to be set by software.
 *             - After the reload register has been set, the current count will be set to the reload value on the next falling edge of the (1.193182 MHz) input signal.
 *             - Subsequent falling edges of the input signal will decrement the current count twice (if the gate input is high on the preceding rising edge of the input signal).
 *             - Note: under normal circumstances the output state will be low 50% of the time when the mode/command register is written. The output will then go high, 
 *               which will generate an immediate (perhaps spurious) IRQ0. The other 50% of the time the output will already be high, and there will be no IRQ0 generated.
 *             - For even reload values, when the current count decrements from two to zero the output of the flop-flop changes state; the current count will be reset 
 *               to the reload value and counting will continue.
 *             - For odd reload values, the current count is always set to one less than the reload value. If the output of the flip flop is low when the current count 
 *               decrements from two to zero it will behave the same as the equivalent even reload value.
 *             - However, if the output of the flip flop is high the reload will be delayed for one input signal cycle (0.8381 uS), which causes the "high" pulse to 
 *               be slightly longer and the duty cycle will not be exactly 50%
 *             - Because the reload value is rounded down to the nearest even number anyway, it is recommended that only even reload values be used (which means you
 *               should mask the value before sending it to the port).
 *             - Note: This even value limitation on the reload value in mode 3 reduces the number of possible output frequencies in half. If you want to be able to 
 *               control the frequency of IRQ0 to a somewhat higher degree, then think about using mode 2 instead for channel 0.
 *             - On channel 2, if the gate input goes low, counting stops and the output goes high immediately. Once the gate input has returned high, the next 
 *               falling edge on input signal will cause the current count to be set to the reload value and operation will continue (with the output left high).
 *             - The reload value can be changed at any time, however the new value will not affect the current count until the current count is reloaded (when 
 *               it is decreased from two to zero, or the gate input going low then high). When this occurs counting will continue using the new reload value.
 *             - A reload value (or divisor) of one must not be used with this mode.
 *           
 *         - Mode 4 – Software Triggered Strobe:
 *             - Mode four operates as a retriggerable delay, and generates a pulse when the current count reaches zero.
 *             - When the mode/command register is written the output signal goes high and the PIT waits for the reload register to be set by software.
 *             - After the reload register has been set, the current count will be set to the reload value on the next falling edge of the (1.193182 MHz) input signal.
 *             - Subsequent falling edges of the input signal will decrement the current count (if the gate input is high on the preceding rising edge of the input signal).
 *             - When the current count decrements from one to zero, the output goes low for one cycle of the input signal (0.8381 uS). The current count will wrap 
 *               around to 0xFFFF (or 0x9999 in BCD mode) and continue to decrement until the mode/command register or the reload register are set, 
 *               however this will not affect the output state.
 *             - If the gate input goes low, counting stops but the output will not be affected and the current count will not be reset to the reload value.
 *             - The reload value can be changed at any time.
 *             - When the new value has been set (both bytes for "lobyte/hibyte" access mode) it will be loaded into the current count on the next falling edge of 
 *               the (1.193182 MHz) input signal, and counting will continue using the new reload value.
 *          
 *         - Mode 5 – Hardware Triggered Strobe:
 *             - Mode 5 is similar to mode 4, except that it waits for the rising edge of the gate input to trigger (or re-trigger) the delay period (like mode 1). 
 *               For this reason it is not usable for PIT channels 0 or 1 (where the gate input can't be changed).
 *             - When the mode/command register is written the output signal goes high and the PIT waits for the reload register to be set by software.
 *             - After the reload register has been set the PIT will wait for the next rising edge of the gate input. Once this occurs, the current count will be 
 *               set to the reload value on the next falling edge of the (1.193182 MHz) input signal.
 *             - Subsequent falling edges of the input signal will decrement the current count.
 *             - When the current count decrements from one to zero, the output goes low for one cycle of the input signal (0.8381 uS). 
 *               The current count will wrap around to 0xFFFF (or 0x9999 in BCD mode) and continue to decrement until the mode/command register or the 
 *               reload register are set, however this will not affect the output state.
 *             - If the gate input signal goes low during this process it will have no effect. However, if the gate input goes high again it will cause the 
 *               current count to be reloaded from the reload register on the next falling edge of the input signal, and restart the count again 
 *               (the same as when counting first started).
 *             - The reload value can be changed at any time, however the new value will not affect the current count until the current count is reloaded 
 *               (on the next rising edge of the gate input). When this occurs counting will continue using the new reload value.
 * 
 *     - COUNTER_LATCH_COMMAND:
 *         - To prevent the current count from being updated, it is possible to "latch" a PIT channel using the latch command.
 *         - To do this, send the value CC000000 (in binary) to the mode/command register (I/O port 0x43), where 'CC' corresponds to the channel number.
 *         - When the latch command has been sent, the current count is copied into an internal "latch register" which can then be read via the data port 
 *           corresponding to the selected channel (I/O ports 0x40 to 0x42). The value kept in the latch register remains the same until it has been fully read, 
 *           or until a new mode/command register is written.
 *         - The main benefit of the latch command is that it allows both bytes of the current count to be read without inconsistencies. For example, 
 *           if you didn't use the latch command, then the current count may decrease from 0x0200 to 0x01FF after you've read the low byte but before you've read 
 *           the high byte, so that your software thinks the counter was 0x0100 instead of 0x0200 (or 0x01FF).
 *         - While the latch command should not affect the current count, on some (old/dodgy) motherboards sending the latch command can cause a cycle of the input 
 *           signal to be occasionally missed, which would cause the current count to be decremented 0.8381ms later than it should be. 
 *           If you're sending the latch command often this could cause accuracy problems (but if you need to send the latch command often you may wish to 
 *           consider redesigning your code anyway).
 * 
 *     - READ_BACK_COMMAND:
 *         - The read back command is a special command sent to the mode/command register (I/O port 0x43).
 *         - The "read back" is not supported on the old 8253 chips but should be supported on all AT and later computers except for PS/2 (i.e. anything that 
 *           isn't obsolete will support it).
 *          _________________________________________________________________________
 *         |   Bits  |                        Usage                                  |
 *         | 7 and 6 | Must be set for the read back command                         |
 *         |    5    | Latch count flag (0 = latch count, 1 = don't latch count)     |
 *         |    4    | Latch status flag (0 = latch status, 1 = don't latch status)  |
 *         |    3    | Read back timer channel 2 (1 = yes, 0 = no)                   |
 *         |    2    | Read back timer channel 1 (1 = yes, 0 = no)                   |
 *         |    1    | Read back timer channel 0 (1 = yes, 0 = no)                   |
 *         |    0    | Reserved (should be clear)                                    |
 *          ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *         - Note: Be careful with bits 4 and 5 – they are inverted.
 *         - Bits 1 to 3 of the read back command select which PIT channels are affected, and allow multiple channels to be selected at the same time.
 *         - If bit 5 is clear, then any/all PIT channels selected with bits 1 to 3 will have their current count copied into their latch register (similar to 
 *           sending the latch command, except it works for multiple channels with one command).
 *         - If bit 4 is clear, then for any/all PIT channels selected with bits 1 to 3, the next read of the corresponding data port will return a 
 *           status byte (discussed below).
 * 
 *     - READ_BACK_STATUS_BYTE:
 *         - After sending a read back command with bit 4 clear, reading the data port for each selected channel will return a status value with the following format:
 *          _____________________________________________________________________
 *          | Bit/s   |                         Usage                             |
 *          |    7    | Output pin state                                          |
 *          |    6    | Null count flags                                          |
 *          | 4 and 5 | Access mode :                                             |
 *          |         |    0 0 = Latch count value command                        |
 *          |         |    0 1 = Access mode: lobyte only                         |
 *          |         |    1 0 = Access mode: hibyte only                         |
 *          |         |    1 1 = Access mode: lobyte/hibyte                       |
 *          | 1 to 3  | Operating mode :                                          |
 *          |         |    0 0 0 = Mode 0 (interrupt on terminal count)           |
 *          |         |    0 0 1 = Mode 1 (hardware re-triggerable one-shot)      |
 *          |         |    0 1 0 = Mode 2 (rate generator)                        |
 *          |         |    0 1 1 = Mode 3 (square wave generator)                 |
 *          |         |    1 0 0 = Mode 4 (software triggered strobe)             |
 *          |         |    1 0 1 = Mode 5 (hardware triggered strobe)             |
 *          |         |    1 1 0 = Mode 2 (rate generator, same as 010b)          |
 *          |         |    1 1 1 = Mode 3 (square wave generator, same as 011b)   |
 *          |    0    | BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD    |
 *           ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *         - The bottom six bits return the values that where programmed into the mode/command register when the channel was last initialized.
 *         - Bit 7 indicates the state of the PIT channel's output pin at the moment that the read-back command was issued.
 *         - Bit 6 indicates whether a newly-programmed divisor value has been loaded into the current count yet (if clear) or the channel is still waiting for a 
 *           trigger signal or for the current count to count down to zero before a newly programmed reload value is loaded into the current count (if set). 
 *           This bit is set when the mode/command register is initialized or when a new reload value is written, and cleared when the reload value is copied into 
 *           the current count.
 */
namespace pit {
    /**
     * @brief PIT - Install the Programmable Interval Timer and perform initial configuration
     *        This register a isr handler in isr.cpp interruptHandlers to receive the timer interruption
     * 
     */
    void install();

    /**
     * @brief PIT - Enable the pit interruptions to be sent to the cpu.
     *              This clear the mask in PIC IRQ0 Line
     * 
     */
    void enable();

    /**
     * @brief PIT - Disable the pit interruptions to be sent to the cpu.
     *              This set the mask in PIC IRQ0 Line
     * 
     */
    void disable();

    /**
     * @brief Configure the given channel with the access, operation and bcd binary mode
     * 
     * @param channel       Channel to be configured
     * @param accessMode    Access mode of this channel
     * @param opMode        Operating Mode of this channel (interrupt on terminal count), (hardware re-triggerable one-shot) ...
     * @param bcdBinMode    BCD/Binary mode. 0=16-bit binary, 1=four-digit BCD
     * @param divisor       Frequency divisor of the timer tick interval. (MIN=0, MAX=65535), smaller_divisor=higher_frequency, and vice-versa. 
     *                      This sets the Reload value of the PIT timer.
     */
    void configureChannel(uint16_t channel, uint8_t accessMode, uint8_t opMode, uint8_t bcdBinMode, uint16_t divisor);

    /**
     * @brief Thread sleep function
     * 
     * @param millis Milliseconds to wait until continue thread execution. 
     *               The accuracy will depend on the divisor frequency of the PIT Channel 0. The higher is divisor the better is accuracy.
     *               The minimum millis depend on the divisor frequency of the PIT Channel 0. For a 0 divisor the minimum millis is 56.
     */
    void sleep(uint32_t millis);

    /**
     * @brief Kernel sleep function
     * 
     * @param millis Milliseconds to wait until continue kernel execution
     *               The accuracy will depend on the divisor frequency of the PIT Channel 0. The higher is divisor the better is accuracy.
     *               The minimum millis depend on the divisor frequency of the PIT Channel 0. For a 0 divisor the minimum millis is 56.
     */
    void ksleep(uint32_t millis);
} 

#endif