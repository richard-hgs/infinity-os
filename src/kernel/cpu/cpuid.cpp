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
const char* INTEL_BRAND_ID_LIST[] = {
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

  // Copy ascii chars from vendorId registers into strVendorId
  memutils::memcpy(strVendorId, vendorId, 12);
  strVendorId[12] = '\0';

  // Print vendor id on screen
  stdio::kprintf("CPUID - VENDOR_ID (0x%x - 0x%x - 0x%x): %s\n", vendorId[0], vendorId[1], vendorId[2], strVendorId);

  // Get additional info about the processor using vendor id
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

    // EAX 80000000h - Get max EAX function value
    cpuid(0x80000000, eax_max, unused, unused, unused); 
    
    // EAX 01h - Get Processor Information
    cpuid(0x01, eax, ebx, ecx, edx); 

    // EAX 01h - EAX Processor signature
    uint8_t steppingId     = eax & 0xF;        // 4 bits: [3:0]             Revision number of the model
    uint8_t model          = eax >> 4 & 0xF;   // 4 bits: [7:4]             Processor model
    uint8_t family         = eax >> 8 & 0xF;   // 4 bits: [11:8]            Processor family
    uint8_t processorType  = eax >> 12 & 0x3;  // 2 bits: [13:12]           Processor Type
                                               // 2 bits: [15:14] reserved
    uint8_t extendedModel  = eax >> 16 & 0xF;  // 4 bits: [19:16]           Processor extended model
    uint8_t extendedFamily = eax >> 20 & 0xFF; // 8 bits: [27:20]           Processor extended family
                                               // 4 bits: [28:31] reserved

    // EAX 01h - EBX Information [APIC_ID, Count, Chunks, BrandID]
    uint8_t brand  = ebx & 0xFF;       // 8 bits: [7:0]      Processor brand id
    uint8_t chunks = ebx >> 8 & 0xFF;  // 8 bits: [15:8]     CFLUSH line size (Value * 8 = cache line size in bytes) 
    uint8_t count  = ebx >> 16 & 0xFF; // 8 bits: [23:16]    Maximum number of logical processors per package, also the number of APIC IDs reserved fo this package. This value doesn't change when processors core are disabled by software. 
    uint8_t apicId = ebx >> 24 & 0xFF; // 8 bits: [31:24]    Default APIC ID

    if (eax_max >= 0x80000004) {
      // Extended Brand string is supported
      // - Brand has 48 chars long.
      // - Since each register is 4 bytes long in x86 and we read from 12 registers this gave to us (48 bytes = 4 * 12)
      uint32_t brandStr[12];

      // Read brand string from registers
      cpuid(0x80000002, brandStr[0], brandStr[1], brandStr[2], brandStr[3]);
      cpuid(0x80000003, brandStr[4], brandStr[5], brandStr[6], brandStr[7]);
      cpuid(0x80000004, brandStr[8], brandStr[9], brandStr[10], brandStr[11]);

      // Print brand string
      // - Since it's null terminated we can cast directly to char* without checking
      stdio::kprintf("CPUID - BRAND: %s", (char*) brandStr);
    } else if (brand > 0) {
      // Extended Brand string unsupported but Brand id is supported
      const char* brandStr = INTEL_BRAND_ID_LIST[brand];

      if (brand == 0x03 && eax == 0x000006B1) {
        // If processor signature = 000006B1h, then Intel(R) Celeron(R) processor
        brandStr = INTEL_BRAND_ID_LIST[1];
      } else if (brand == 0x0B && eax == 0x00000F13) {
        // If processor signature = 00000F13h, then Intel(R) Xeon(R) processor MP
        brandStr = INTEL_BRAND_ID_LIST[12];
      } else if (brand == 0x0E && eax == 0x00000F13) {
        // If processor signature = 00000F13h, then Intel(R) Xeon(R) processor
        brandStr = INTEL_BRAND_ID_LIST[11];
      }

      // Print brand string gathered from INTEL_BRAND_ID_LIST
      stdio::kprintf("CPUID - BRAND: %s - REVISION: %x", INTEL_BRAND_ID_LIST[brand]);

    } /* else { If brand is unsupported should use processor signature in conjunction with cache descriptors to identify the processor */
    
    stdio::kprintf(" - REVISION: %x\n", steppingId);
    stdio::kprintf("CPUID - CACHE: %d bytes - CORES: %d - APIC_ID: %x\n", chunks * 8, count, apicId);
}