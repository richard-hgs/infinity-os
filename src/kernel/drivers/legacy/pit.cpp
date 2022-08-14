// cpu
#include "pic.h"
#include "isr.h"
// stdlib
#include "stdio.h"
#include "stdlib.h"
// sys
#include "io.h"
#include "pit.h"

#define PIT_CRYSTAL_FREQUENCY 1193182       // The PIT contains a crystal oscillator which emits a signal 1193182 hz.
#define MIN_FREQ_DIVISOR            1       // 1 is the min, because 0 is interpreted as 65536
#define MAX_FREQ_DIVISOR        65535       // 65535 is the max because the divisor is 16 bits long and the MAX 16 bit value is 65535.

#define IO_CHANNEL_0             0x40       // Channel 0 (Read/Write) The output from PIT channel 0 is connected to the PIC chip, so that it generates an "IRQ 0". Infinite timer ticks.
#define IO_CHANNEL_1             0x41       // Channel 1 (Read/Write) In OLD days channel 1 was used to refresh DRAM wich was composed with tiny capacitors that needed to get voltage to keep data stored, since capacitors lose their current over time until reach 0.
#define IO_CHANNEL_2             0x42       // Channel 2 (Read/Write) Connected to PC Speaker. So the frequency of the output determines the frequency of the sound produced by the speaker. Controlled by software (via bit 0 of I/O port 0x61). Output (a high or low voltage) can be read by software (via bit 5 of I/O port 0x61)

#define IO_CR                    0x43       // Mode/Command register (write only, a read is ignored)

/**
 * @brief Select channel (Bits 7-6)
 * 
 */
#define CMD_CHANNEL_0             0x0       // Select channel 0 to configure
#define CMD_CHANNEL_1             0x1       // Select channel 1 to configure
#define CMD_CHANNEL_2             0x2       // Select channel 2 to configure
#define CMD_CHANNEL_READ_BACK     0x3       // Read-back command (8254 only)

/**
 * @brief Access Mode (Bits 5-4)
 * 
 */
#define CMD_AM_LATCH_COUNT        0x0       // Latch count value command
#define CMD_AM_LOBYTE_ONLY        0x1       // Access mode: lobyte only
#define CMD_AM_HIBYTE_ONLY        0x2       // Access mode: hibyte only
#define CMD_AM_LO_HI_BYTE         0x3       // Access mode: lobyte/hibyte

/**
 * @brief Operating Mode (Bits 3-1)
 * 
 */
#define CMD_OPMODE_INT_ON_TERM_COUNT     0x0       // Interrupt on terminal count
#define CMD_OPMODE_ONE_SHOT              0x1       // Hardware re-triggerable one-shot
#define CMD_OPMODE_RATE_GEN              0x2       // Rate generator
#define CMD_OPMODE_SQUARE_WAVE           0x3       // Square wave generator
#define CMD_OPMODE_SOFTWARE_STROBE       0x4       // Software triggered strobe
#define CMD_OPMODE_HARDWARE_STROBE       0x5       // Hardware triggered strobe
#define CMD_OPMODE_RATE_GEN_2            0x6       // Rate generator, same as 010b
#define CMD_OPMODE_SQUARE_WAVE_2         0x7       // Square wave generator, same as 011b

/**
 * @brief BCD/Binary mode (Bit 0)
 * 
 */
#define CMD_BCD_16_BIT                   0x0       // 16 Bit (Min=0, Max=65535)
#define CMD_BCD_4_DIGIT                  0x1       // 4 decimal digits (Min=0, Max=9999)

#define TIMER_BLOCK_BUF_SIZE 2                     // Amount of countdown timers available to user and driver threads

uint32_t channel0Divisor;

struct TimerBlock {
    uint8_t id;
    uint32_t countdown;
} timerBlocks[TIMER_BLOCK_BUF_SIZE];

uint32_t kCountdownTimer; // Kernel countdown timer

void timerInterruptHandler(registers_t* r) {
    // On each tick exchange the user process
    // In case of one countdown timer finishes change to this process instead
    if (kCountdownTimer > 0) {  // Decrement kernel countdown timer until reaches 0.
        kCountdownTimer--;
    }
}

void pit::install() {
    uint8_t i;
    // Zero fill .bss unitialized data. Must be initialized.
    for (i=0; i<TIMER_BLOCK_BUF_SIZE; i++) {
        timerBlocks[i].id = 0;
        timerBlocks[i].id = 0;
    }

    // Setup the handler
    isr::registerIsrHandler(IRQ0, timerInterruptHandler);

    // Configure pit channel to lowest frequency possible
    configureChannel(IO_CHANNEL_0, CMD_AM_LO_HI_BYTE, CMD_OPMODE_SQUARE_WAVE, CMD_BCD_16_BIT, 1193);
}

void pit::enable() {
    pic::clearMask(0); // 0 = IRQ0
}

void pit::disable() {
    pic::setMask(0);   // 0 = IRQ0
}

void pit::configureChannel(uint16_t channel, uint8_t accessMode, uint8_t opMode, uint8_t bcdBinMode, uint16_t divisor) {
    uint32_t mDivisor = 0;
    uint8_t low = 0;
    uint8_t high = 0;
    uint8_t command = 0; // Command to be sent to the PIT

    if (divisor >= MIN_FREQ_DIVISOR && divisor <= MAX_FREQ_DIVISOR) { // Calculates the divisor of the PIT frequency
        mDivisor = PIT_CRYSTAL_FREQUENCY / divisor;
        low  = (uint8_t)(mDivisor & 0xff);
        high = (uint8_t)((mDivisor >> 8) & 0xff);
        channel0Divisor = mDivisor;
    } else {
        channel0Divisor = 18; // 0=18hz
    }

    command = channel & 0x3;
    command = (command << 2) | (accessMode & 0x3);
    command = (command << 3) | (opMode & 0x7);
    command = (command << 1) | (bcdBinMode & 0x1);
    
    stdio::kprintf("");

    // Issue the commands
    io::outb(IO_CR, command);
	io::outb(channel, low);
	io::outb(channel, high);
}

TimerBlock* findTimerBlock() {
    int i;
    for (i=0; i<TIMER_BLOCK_BUF_SIZE; i++) {
        if (timerBlocks[i].id == NULL) {
            timerBlocks->id = i + 1;    // Id - index of this timer
            return &timerBlocks[i];
        }
    }
    return NULL;
}

void pit::sleep(uint32_t millis) {
    struct TimerBlock* t;
    if ((t = findTimerBlock()) == NULL) { // All timer blocks in use by the processes
        return;
    }

    // We need to perform a rule of 3 to know how many ticks are necessary to decrement a given amount of milliseconds.
    // channel0Divisor ----> 1000 millis
    // kCountdownTimer ----> millis
    t->countdown = millis * channel0Divisor / 1000;

}

void pit::ksleep(uint32_t millis) {
    // We need to perform a rule of 3 to know how many ticks are necessary to decrement a given amount of milliseconds.
    // channel0Divisor ----> 1000 millis
    // kCountdownTimer ----> millis
    kCountdownTimer = millis * channel0Divisor / 1000;
    while(kCountdownTimer > 0) {
        __asm__ volatile ("hlt"); // Halt the cpu. Waits until an IRQ occurs minimize CPU usage
    } // Wait until countdown timer reaches 0 then continue execution.
}

/**
 * @brief TSC - Time Stamp Counter
 * 
 * The instruction RDTSC returns the TSC in EDX:EAX. 
 * In x86-64 mode, RDTSC also clears the upper 32 bits of RAX and RDX. 
 * Its opcode is 0F 31
 */