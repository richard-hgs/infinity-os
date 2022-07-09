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

/**
 * @brief Brand ID (EAX=1) Return Values in EBX (Bits [9:7])
 * 
 */
char* INTEL_BRAND_ID_LIST[] = {
  /* 00h */ "Unsupported",
  /* 01h */ "Intel(R) Celeron(R) processor",
  /* 02h */ "Intel(R) Pentium(R) III processor",
  /* 03h */ "Intel(R) Pentium(R) III Xeon(R) processor", // If processor signature = 000006B1h, then Intel(R) Celeron(R) processor
  /* 04h */ "Intel(R) Pentium(R) III processor",
  /* 05h */ "Reserved",
  /* 06h */ "Mobile Intel(R) Pentium(R) III processor-M",
  /* 07h */ "Mobile Intel(R) Celeron(R) processor",
  /* 08h */ "Intel(R) Pentium(R) 4 processor",
  /* 09h */ "Intel(R) Pentium(R) 4 processor",
  /* 0Ah */ "Intel(R) Celeron(R) processor",
  /* 0Bh */ "Intel(R) Xeon(R) processor", // If processor signature = 00000F13h, then Intel(R) Xeon(R) processor MP
  /* 0Ch */ "Intel(R) Xeon(R) processor MP",
  /* 0Dh */ "Reserved",
  /* 0Eh */ "Mobile Intel(R) Pentium(R) 4 processor-M", // If processor signature = 00000F13h, then Intel(R) Xeon(R) processor
  /* 0Fh */ "Mobile Intel(R) Celeron(R) processor",
  /* 10h */ "Reserved",
  /* 11h */ "Mobile Genuine Intel(R) processor",
  /* 12h */ "Intel(R) Celeron(R) M processor",
  /* 13h */ "Mobile Intel(R) Celeron(R) processor",
  /* 14h */ "Intel(R) Celeron(R) processor",
  /* 15h */ "Mobile Genuine Intel(R) processor",
  /* 16h */ "Intel(R) Pentium(R) M processor",
  /* 17h */ "Mobile Intel(R) Celeron(R) processor"
  // All other values - Reserved
};

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
    stdio::kprintf("%s\n", INTEL_BRAND_ID_LIST[1]);
}