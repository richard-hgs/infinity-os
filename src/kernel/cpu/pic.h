#pragma once
#ifndef _PIC_H_
#define _PIC_H_

// libc
#include <stdint.h>

/**
 * @brief 8259 Programmable Interrupt Controller (PIC)
 * 
 * MASTER_PIC: 
 *     - Vector offset: 0x08
 *     - Interrupt Numbers: 0x08 to 0x0F
 *     - IRQ_0 - IRQ_7:
 *         - IRQ_0: System Timer (cannot be changed)
 *         - IRQ_1: Keyboard on PS/2 port (cannot be changed)
 *         - IRQ_2: Cascaded signals from IRQ_8 - IRQ_15 (any devices configured to use IRQ_2 will actually be using IRQ_9)
 *         - IRQ_3: Serial port controller for serial port 2 (shared with serial port 4, if present)
 *         - IRQ_4: Serial port controller for serial port 1 (shared with serial port 3, if present)
 *         - IRQ_5: Parallel port 3 or sound card
 *         - IRQ_6: Floppy disk controller
 *         - IRQ_7: Parallel port 1 (shared with parallel port 2, if present). It is used for printers or for any parallel port if a printer is not present. It can also be potentially be shared with a secondary sound card with careful management of the port.
 * 
 * SLAVE_PIC:
 *     - Vector offset: 0x70
 *     - Interrupt Numbers: 0x70 to 0x77
 *     - IRQ_8 - IRQ_15:
 *         - IRQ_8: Real time clock (RTC)
 *         - IRQ_9: Advanced Configuration and Power Interface (ACPI) system control interrupt on Intel chipsets. Other chipset manufactures might use another interrupt for this purpose, or make it available for the use of peripherals (any devices configured to use IRQ_2 will actually be using IRQ_9)
 *         - IRQ_10: The interrupt is left open for the use of peripherals (open interrupt/available, SCSI or NIC)
 *         - IRQ_11: The interrupt is left open for the use of peripherals (open interrupt/available, SCSI or NIC)
 *         - IRQ_12: Mouse on PS/2
 *         - IRQ_13: CPU co-processor or integrated floating point unit or inter-processor interrupt (use depends on OS)
 *         - IRQ_14: Primary ATA channel (ATA interface usually serves hard disk drives and CD drives)
 *         - IRQ_15: Secondary ATA channel
 * 
 * - Since IRQs 0 to 7 conflict with the CPU exceptions which are reserved by Intel up until 0x1F. We need to remap the IRQs offset in memory.
 * 
 * - Interrupt Mask Register (IMR) 
 *     The PIC has an internal register called the IMR, or the Interrupt Mask Register. It is 8 bits wide. 
 *     This register is a bitmap of the request lines going into the PIC. When a bit is set, the PIC ignores the request and 
 *     continues normal operation. Note that setting the mask on a higher request line will not affect a lower line. 
 *     Masking IRQ2 will cause the Slave PIC to stop raising IRQs.
 */
namespace pic {
    /**
     * @brief Remps the PIC vectors offsets to a new given offset
     * 
     * @param offset1 New MASTER PIC vector offset
     * @param offset2 New SLAVE PIC vector offset
     */
    void remap(int offset1, int offset2);

    /**
     * @brief Disable the 8259 PIC
     * 
     */
    void disable();

    /**
     * @brief Interrupt Mask Register (IMR) 
     *        Set a mask to given irq line
     * 
     * @param irql Irq line to be masked
     */
    void setMask(uint8_t irql);

    /**
     * @brief Interrupt Mask Register (IMR) 
     *        Clear the mask of the given irql line
     * 
     * @param irql Irq line to be unmasked
     */
    void clearMask(uint8_t irql);

    /**
     * @brief End of Interrupt (EOI)
     *        Reset the In Service (IS) bit for given PIC IRQ line
     * 
     * @param irq 
     */
    void sendEOI(uint8_t irq);
}

#endif