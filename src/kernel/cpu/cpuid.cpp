// stdlibs
#include "stdio.h"
// memory
#include "memutils.h"
#include "cpuid.h"

/**
 * @brief CPUID FUNCTION
 * Input a value in the EAX register
 * Execute the cpuid instruction
 * Output the value of REGISTERS EAX, EBX, ECX, EDX into given variables
 * 
 */
#define cpuid(in, a, b, c, d) __asm__("cpuid": "=a" (a), "=b" (b), "=c" (c), "=d" (d) : "a" (in));

void getIntelCpuInfo();

void cpuid::detectCpu() {
  uint32_t vendorId[3];
  uint32_t maxEax;
  char strVendorId[13];

  // EAX = 0: MAX_EAX, VENDOR_ID
  cpuid(0, maxEax, vendorId[0], vendorId[2], vendorId[1]);

  memutils::memcpy(strVendorId, vendorId, 12);
  strVendorId[12] = '\0';

  stdio::kprintf("CPUID - VENDOR_ID (0x%x - 0x%x - 0x%x): %s\n", vendorId[0], vendorId[1], vendorId[2], strVendorId);

  switch(vendorId[0]) {
    case 0x756e6547: // Intel vendor id
      getIntelCpuInfo();
      break;
    default:
      stdio::kprintf("Unknown x86 CPU detected.\n");
      break;
  }
}

void getIntelCpuInfo() {
    uint32_t eax_max;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t unused;

    cpuid(0x80000000, eax_max, unused, unused, unused); // Get the eax max input value
    
    cpuid(1, eax, ebx, ecx, edx); // Get Old processors information using table list

    stdio::kprintf("CPUID - eax: %x - ebx: %x - ecx: %x - edx: %x\n", eax, ebx, ecx, edx);
    
    // CPUID - EAX Information
    uint8_t steppingId     = eax & 0xF;        // 4 bits: [3:0]
    uint8_t model          = eax >> 4 & 0xF;   // 4 bits: [7:4]
    uint8_t family         = eax >> 8 & 0xF;   // 4 bits: [11:8]
    uint8_t processorType  = eax >> 12 & 0x3;  // 2 bits: [13:12]
                                               // 2 bits: [15:14] reserved
    uint8_t extendedModel  = eax >> 16 & 0xF;  // 4 bits: [19:16]
    uint8_t extendedFamily = eax >> 20 & 0xFF; // 8 bits: [27:20]
                                               // 4 bits: [28:31] reserved

    stdio::kprintf("CPUID - steppingId: %x - model: %x - family: %x - processorType: %x\n", steppingId, model, family, processorType);
    stdio::kprintf("CPUID - extendedModel: %x - extendedFamily: %x\n", extendedModel, extendedFamily);
}