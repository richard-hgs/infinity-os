#pragma once
#ifndef _APIC_H_
#define _APIC_H_

#define APIC_NO_ERROR 0            // No error happend. Same as Success
#define APIC_ERROR_NOT_PRESENT 1   // Cpu don't have an APIC

/**
 * @brief APIC - Advanced Programmable Interrupt Controller
 * 
 * DETECTING_APIC_PRESENCE
 *     - Beginning with the P6 family processors, the presence or absence of an on-chip local 
 *     APIC can be detected using the CPUID instruction. When the CPUID instruction is 
 *     executed with a source operand of 1 in the EAX register, bit 9 of the CPUID feature 
 *     flags returned in the EDX register indicates the presence (set) or absence (clear) of a 
 *     local APIC.
 * 
 * 
 */
namespace apic {
    /**
     * @brief APIC install and configure if present
     * 
     * @return int 0=NO_ERROR, >0=ERROR_CODE
     */
    int install();
}

#endif