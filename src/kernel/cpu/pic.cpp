// sys
#include "io.h"
#include "pic.h"

#define PIC1		0x20		// 0x20 IO base address for master PIC
#define PIC2		0xA0		// 0x21 IO base address for slave PIC
#define PIC1_COMMAND	PIC1    // 0x20 MASTER PIC command port
#define PIC1_DATA	(PIC1+1)    // 0x21 MASTER PIC data port
#define PIC2_COMMAND	PIC2    // 0xA0 SLAVE PIC command port
#define PIC2_DATA	(PIC2+1)    // 0xA1 SLAVE PIC data port

#define PIC_EOI		0x20		// End-of-interrupt command code

// Used to reinitialize the PIC controllers, giving them specified vector offsets
// rather than 8h and 70h, as configured by default
 
#define ICW1_ICW4	    0x01		// ICW4 (not) needed
#define ICW1_SINGLE	    0x02		// Single (cascade) mode
#define ICW1_INTERVAL4	0x04		// Call address interval 4 (8)
#define ICW1_LEVEL	    0x08		// Level triggered (edge) mode
#define ICW1_INIT	    0x10		// Initialization - required!
 
#define ICW4_8086	    0x01		// 8086/88 (MCS-80/85) mode
#define ICW4_AUTO	    0x02		// Auto (normal) EOI
#define ICW4_BUF_SLAVE	0x08		// Buffered mode/slave
#define ICW4_BUF_MASTER	0x0C		// Buffered mode/master
#define ICW4_SFNM	    0x10		// Special fully nested (not)

#define PIC_READ_IRR    0x0a        // OCW3 irq ready next CMD read
#define PIC_READ_ISR    0x0b        // OCW3 irq service next CMD read

/**
 * @brief Get IRQ register through OCW3
 * 
 * @param ocw3      The PICs register being readed
 * @return uint16_t Registers from PIC_2 (Bits [15:8]) and PIC_1 (Bits [7:0])
 */
uint16_t getIrqReg(int ocw3);

/**
 * @brief Get the Interrupt Request Register (IRR)
 * 
 * @return uint16_t IRR Registers from PIC_2 (Bits [15:8]) and PIC_1 (Bits [7:0])
 */
uint16_t getIrr();

/**
 * @brief Get the In-Service Register (ISR)
 * 
 * @return uint16_t IRR Registers from PIC_2 (Bits [15:8]) and PIC_1 (Bits [7:0])
 */
uint16_t getIsr();

void pic::disable() {
    io::outb(PIC2_DATA, 0xff);
    io::outb(PIC1_DATA, 0xff);
}

void pic::setMask(uint8_t irql) {
    uint16_t port;
    uint8_t value;
 
    if(irql < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irql -= 8;
    }
    value = io::inb(port) | (1 << irql);
    io::outb(port, value);   
}

void pic::clearMask(uint8_t irql) {
    uint16_t port;
    uint8_t value;
 
    if(irql < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irql -= 8;
    }
    value = io::inb(port) & ~(1 << irql);
    io::outb(port, value);        
}

void pic::remap(int offset1, int offset2) {
    unsigned char a1, a2;
 
	a1 = io::inb(PIC1_DATA);                        // save masks
	a2 = io::inb(PIC2_DATA);
 
	io::outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io::wait();
	io::outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io::wait();
	io::outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io::wait();
	io::outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io::wait();
	io::outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io::wait();
	io::outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io::wait();
 
	io::outb(PIC1_DATA, ICW4_8086);
	io::wait();
	io::outb(PIC2_DATA, ICW4_8086);
	io::wait();
 
	io::outb(PIC1_DATA, a1);   // restore saved masks.
	io::outb(PIC2_DATA, a2);
}

void pic::sendEOI(uint8_t irq) {
	if(irq >= 8) {
		io::outb(PIC2_COMMAND, PIC_EOI); // Reset the In Service (IS) bit for the Slave PIC cascading it to the Master PIC
    }
	io::outb(PIC1_COMMAND, PIC_EOI); // Reset the In Service (IS) bit for the Master PIC
}

uint16_t getIrqReg(int ocw3) {
    /* OCW3 to PIC CMD to get the register values.  PIC2 is chained, and
     * represents IRQs 8-15.  PIC1 is IRQs 0-7, with 2 being the chain */
    io::outb(PIC1_COMMAND, ocw3);
    io::outb(PIC2_COMMAND, ocw3);
    return (io::inb(PIC2_COMMAND) << 8) | io::inb(PIC1_COMMAND);   
}

uint16_t getIrr() {
    return getIrqReg(PIC_READ_IRR);
}

uint16_t getIsr() {
    return getIrqReg(PIC_READ_ISR);
}