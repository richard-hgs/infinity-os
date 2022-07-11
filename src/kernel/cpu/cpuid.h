#pragma once
#ifndef _CPUID_H_
#define _CPUID_H_

#include <stdint.h>
#include <stdbool.h>

#define CPUID_UNKNOW_CPU 0
#define CPUID_INTEL 1
#define CPUID_AMD 2

/**
 * @brief CPUID - Specification
 * 
 * VENDOR_ID:
 *    - Execute the CPUID instruction with EAX equal to 0h.
 *    - The register will return "GenuineIntel" ASCII string
 *    - EAX will return the largest function number supported.
 *    - EBX will return vendor_id 4 bytes [(u) 0x75, (n) 0x6E (e) 0x65, (G) 0x47]
 *    - EDX will return vendor_id 4 bytes [(I) 0x49, (e) 0x65 (n) 0x6E, (i) 0x69]
 *    - ECX will return vendor_id 4 bytes [(l) 0x6C, (e) 0x65 (t) 0x74, (n) 0x6E]
 * 
 * BRAND_STRING_ID_SUPPORTED:
 *    - Execute the CPUID instruction with EAX equal to 80000000h.
 *    - If CPUID returns EAX greather than or equal to 80000004h the brand string feature is supported.
 *    - If supported use CPUID functions 80000002h through 80000004h to identify the processor.
 *    - Using the brand string feature, future IA-32 architecture based processors will return their ASCII brand identification and
 *      maximum operating frequency via an extended CPUID instruction.
 *    - When CPUID is executed with EAX:
 *       - 80000000h - Largest Extended Function Supported;
 *                   - EAX = Largest supported extended function number;
 *                   - EBX = ECX = EDX = Reserved;
 * 
 *       - 80000001h - Extended Processor Signature and Extended Feature Bits;
 *                   - EDX and ECX contain Extended Feature Flags
 *                   - EAX = EBX = Reserved;
 * 
 *       - 80000002h - Processor Brand String;
 *                   - EAX, EBX, ECX, EDX contain ASCII brand string
 * 
 *       - 80000003h - Processor Brand String;
 *                   - EAX, EBX, ECX, EDX contain ASCII brand string
 * 
 *       - 80000004h - Processor Brand String;
 *                   - EAX, EBX, ECX, EDX contain ASCII brand string
 * 
 * BRAND_STRING_ID_NOT_SUPPORTED:
 *    - Execute the CPUID instruction with eax equal to 1.
 *    - Returns the ProcessorSignature in the EAX register.
 *    - BrandId in the EBX register bits 0 through 7.
 *    - If the EBX register bits 0 through 7 contain a non zero value, the BrandId is supported.
 *    - Software should scan the list of Brand IDs to identify the processor.
 * 
 * BRAND_ID_NOT_SUPPORTED:
 *    - Software should use the processor signature in conjunction with cache descriptors to identify the processor.
 *     
 * MAX_EAX_INPUT_SUPPORTED:
 *    - Set EAX register with value 80000000h and execute the CPUID instruction.
 *    - The max EAX value that can be used with CPUID instruction will be stored in EAX register.
 *    - The CPUID instruction can't receive EAX input values greather than the max EAX value.
 */
namespace cpuid {
    /**
     * @brief Detect cpu type and print its information
     * 
     */
    void printCpuInfo();

    /**
     * @brief Get current CPU type
     * 
     * @return uint8_t 0=CPUID_UNKNOW_CPU, 1=CPUID_INTEL, 2=CPUID_AMD
     */
    uint8_t detectCpu();

    /**
     * @brief Return whether the APIC is present or not in the processor
     * 
     * @return true  APIC is present and can be used
     * @return false APIC is not present and can't be used
     */
    bool hasApic();
}

#endif