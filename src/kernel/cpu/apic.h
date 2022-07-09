#pragma once
#ifndef _APIC_H_
#define _APIC_H_

/**
 * @brief APIC - Advanced Programmable Interrupt Controller
 * 
 * DETECTING_APIC_PRESENCE
 *     - Beginning with the P6 family processors, the presence or absence of an on-chip local 
 *     APIC can be detected using the CPUID instruction. When the CPUID instruction is 
 *     executed with a source operand of 1 in the EAX register, bit 9 of the CPUID feature 
 *     flags returned in the EDX register indicates the presence (set) or absence (clear) of a 
 *     local APIC.
 */
namespace apic {

}

#endif