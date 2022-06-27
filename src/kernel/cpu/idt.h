#pragma once
#ifndef _IDT_H_
#define _IDT_H_

// // #include "types.h"
#include <stdint.h>

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
    void setGate(uint8_t i, uint32_t handler);

    /**
	 * @brief Install a new IDT into CPU system
     * This allow kernel to handle Hardware interruptions and exceptions
     * 
	 */
    void install(void);
}
#endif