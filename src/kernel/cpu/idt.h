#pragma once
#ifndef _IDT_H_
#define _IDT_H_

// // #include "types.h"
#include <stdint.h>

#define IDT_ENTRIES 256                         // Max idt entries of the idt buffer 0-254=255 idt entries

#define IDT_KERNEL_CS           0x08            // 0x08 is a stand-in to our kernel code segment in GDT table

// FLAGS - Bit(7) gate presence
#define IDT_GATE_PRESENT        0x01
#define IDT_GATE_NOT_PRESENT    0x00

// FLAGS - Bits(6-5) gate cpu privilege levels
#define IDT_GATE_DPL_PRIVILEGE_MAX      0x00    // Can access any segment on the system
#define IDT_GATE_DPL_PRIVILEGE_HIGH     0x01    // Can access HIGH, NORMAL AND LOW segments on the system
#define IDT_GATE_DPL_PRIVILEGE_NORMAL   0x02    // Can access NORMAL AND LOW segments on the system
#define IDT_GATE_DPL_PRIVILEGE_LOW      0x03    // Can access only LOW segments on the system

// FLAGS - Bit(4) gate storage segment
#define IDT_GATE_STORAGE_SEG_INT        0x00    // Must be 0 for interruptions gates

// FLAGS - Bit(3-0) gate type
#define IDT_GATE_TYPE_X86_TASK          0x05    // Task gate type
#define IDT_GATE_TYPE_X86_INTERRUPT     0x0E    // Interruption gate type
#define IDT_GATE_TYPE_X86_TRAP          0x0F    // Trap gate type

/** 
 *   Table 5-1. Relationship of the IDTR and IDT
 *              docs/intel_x86_x64_specification.pdf page 204-205
 * 
 *  - IDT Interrupt Gate Specification:
 *   _______________________________________________________________
 *  | 31 --------------------- 16 | 15 | 14-13 | 12-8   | 7-5 | 4-0 |
 *  | Offset Entry Point[16:31]   | P  | DPL   | 0[D]110| 000 | RES |
 *  |-----------------------------|---------------------------------|
 *  | Segment Selector            | Offset Entry Point[0:15]        |
 *   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 * - Offset Entry Point:
 *      - Offset to procedure entry point;
 * 
 * - Segment Selector:
 *      - Segment Selector for destination code segment of GDT;
 * 
 * - RES :
 *      - Intel reserved bits;
 * 
 * - D :
 *      -  Size of gate 1 = 32 bits; 0 = 16 bits.
 * 
 * - DPL :
 *      - Descriptor Privilege Level
 *      - Privilege levels range from 0-3; lower numbers are more privileged.
 *      - CPU Privilege Levels which are allowed to access this interrupt via the INT instruction. 
 *      - Hardware interrupts ignore this mechanism.
 * 
 * - P : 
 *     - Segment Present flag
 *     - If clear, a "segment not present" exception is generated on any reference to this segment.
 *       Descriptor is undefined.
 *     - If 1, descriptor contains a valid base and limit.
 * 
 *   Table 5-1. Protected-Mode Exceptions and Interrupts
 *              docs/intel_x86_x64_specification.pdf page 193
 *    ______________________________________________________________________________________________________________________________________________________________________
 *   | VCTOR NO. |              DESCRIPTION                         |      TYPE      |  ERROR CODE  |           SOURCE                                                      |
 *   |     0     | DE - Divide Error                                |      Fault     |      No      | DIV and IDIV instructions.                                            |
 *   |     1     | DB - Reserved                                    |  Fault / Trap  |      No      | For intel use only.                                                   |
 *   |     2     | -- - NMI Interrupt                               |    Interrupt   |      No      | Non maskable external interrupt.                                      |
 *   |     3     | BP - Break Point                                 |      Trap      |      No      | Int 3 instruction.                                                    |
 *   |     4     | OF - Overflow                                    |      Trap      |      No      | INTO instruction.                                                     |
 *   |     5     | BR - Bound Range Exceeded                        |      Fault     |      No      | BOUND instruction.                                                    |
 *   |     6     | UD - Invalid Opcode (Undefined Opcode)           |      Fault     |      No      | UD2 instruction or reserved opcode.                                   |
 *   |     7     | NM - Device not available (No Math Coprocessor)  |      Fault     |      No      | Floating-point or WAIT/FWAIT instruction.                             |
 *   |     8     | DF - Double Fault                                |      Abort     |  Yes (zero)  | Any instruction that can generate an exception, an NMI, or an INTR    |
 *   |     9     | -- - Coprocessor Segment Overrun (reserved)      |      Fault     |      No      | Floating-point instruction.                                           |
 *   |    10     | TS - Invalid TSS                                 |      Fault     |      Yes     | Task switch or TSS access.                                            |
 *   |    11     | NP - Segment Not Present                         |      Fault     |      Yes     | Loading segment registers or accessing system segments.               |
 *   |    12     | SS - Stack-Segment Fault                         |      Fault     |      Yes     | Stack operations and SS register loads.                               |
 *   |    13     | GP - General Protection                          |      Fault     |      Yes     | Any memory reference and other protection checks.                     |
 *   |    14     | PF - Page Fault                                  |      Fault     |      Yes     | Any memory reference.                                                 |
 *   |    15     | -- - Intel Reserved (Do not use)                 |      -----     |      No      |                                                                       |
 *   |    16     | MF - x87 FPU Floating-Point Error (Math Fault)   |      Fault     |      No      | x87 FPU floating-point or WAIT/FWAIT instruction.                     |
 *   |    17     | AC - Alignment Check                             |      Fault     |  Yes (Zero)  | Any data reference in memory.                                         |
 *   |    18     | MC - Machine Check                               |      Abort     |      No      | Error codes (if any) and source are model dependent.                  |
 *   |    19     | XM - SIMD Floating-Point Exception               |      Fault     |      No      | SSE/SSE2/SSE3 floating-point instructions.                            |
 *   |   20-31   | -- - Intel reserved. Do not use                  |      -----     |      --      |                                                                       |
 *   |   32-255  | -- - User Defined (Non-reserved) Interrupts      |    Interrupt   |      --      | External interrupt or INT n instruction.                              |
 *    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 */

/* Interrupt gate handler definition. */
typedef struct idt_gate {
    uint16_t low_offset;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t high_offset;
} __attribute__((packed)) idt_gate_t;

/* A point to the array of interrupt handlers. */
typedef struct idt_reg {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_reg_t;

namespace idt {
    /**
     * @brief Set the IDT handlers one at a time. Since they aren't linear we need to perform bitwise operations in 32, 16 and 8 bits 
     * variables to achieve a IDT table entry.
     * 
     * @param i         IDT handler index
     * @param handler   IDT handler function ptr address
     */
    void setGate(uint16_t i, uint32_t handler, uint16_t cs, uint8_t present, uint8_t dpl, uint8_t storageSegment, uint8_t gateType);

    /**
	 * @brief Install a new IDT into CPU system
     * This allow kernel to handle Hardware interruptions and exceptions
     * 
	 */
    void install(void);
}
#endif