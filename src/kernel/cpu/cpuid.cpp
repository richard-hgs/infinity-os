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

typedef struct {
  uint8_t exFamily;
  uint8_t exModel;
  uint8_t type;
  uint8_t family;
  uint8_t model;
  uint8_t stepping;
  const char** strArch;
} IntelSignature;

const char* DEFAULT_STR = "N/D";

/**
 * @brief Processor Type (EAX=1) Return values in EAX (Bits [13:12])
 * 
 */
const char* INTEL_PROCESSOR_TYPE_LIST[] = {
  /* 00h */ "Original OEM Processor",
  /* 01h */ "OverDrive(R) Processor",
  /* 02h */ "Dual Processor",
  /* 03h */ "Intel Reserved",
};

/**
 * @brief Brand ID (EAX=1) Return Values in EBX (Bits [9:7])
 * 
 */
const char* INTEL_BRAND_ID_LIST[] = {
  /* 00h */ "Unsupported",
  /* 01h */ "Intel(R) Celeron(R) Processor",
  /* 02h */ "Intel(R) Pentium(R) III Processor",
  /* 03h */ "Intel(R) Pentium(R) III Xeon(R) Processor", // If processor signature = 000006B1h, then Intel(R) Celeron(R) processor
  /* 04h */ "Intel(R) Pentium(R) III Processor",
  /* 05h */ "Reserved",
  /* 06h */ "Mobile Intel(R) Pentium(R) III Processor-M",
  /* 07h */ "Mobile Intel(R) Celeron(R) Processor",
  /* 08h */ "Intel(R) Pentium(R) 4 Processor",
  /* 09h */ "Intel(R) Pentium(R) 4 Processor",
  /* 0Ah */ "Intel(R) Celeron(R) Processor",
  /* 0Bh */ "Intel(R) Xeon(R) Processor", // If processor signature = 00000F13h, then Intel(R) Xeon(R) processor MP
  /* 0Ch */ "Intel(R) Xeon(R) Processor MP",
  /* 0Dh */ "Reserved",
  /* 0Eh */ "Mobile Intel(R) Pentium(R) 4 Processor-M", // If processor signature = 00000F13h, then Intel(R) Xeon(R) processor
  /* 0Fh */ "Mobile Intel(R) Celeron(R) Processor",
  /* 10h */ "Reserved",
  /* 11h */ "Mobile Genuine Intel(R) Processor",
  /* 12h */ "Intel(R) Celeron(R) M Processor",
  /* 13h */ "Mobile Intel(R) Celeron(R) Processor",
  /* 14h */ "Intel(R) Celeron(R) Processor",
  /* 15h */ "Mobile Genuine Intel(R) Processor",
  /* 16h */ "Intel(R) Pentium(R) M Processor",
  /* 17h */ "Mobile Intel(R) Celeron(R) Processor"
  // All other values - Reserved
};

const char* INTEL_ARCH_LIST[] = {
  /* 00h */ "i386",                                                                     // Family 3
  /* 01h */ "i486",                                                                     // Family 4
  /* 02h */ "P5",                                                                       // Family 5
  /* 03h */ "Lakemont",                                                                 // Family 5
  /* 04h */ "Knights Mill",                                                             // Family 6
  /* 05h */ "Knights Landing",                                                          // Family 6
  /* 06h */ "Bonnel",                                                                   // Family 6
  /* 07h */ "Saltwell",                                                                 // Family 6
  /* 08h */ "Silvermont",                                                               // Family 6
  /* 09h */ "Airmont",                                                                  // Family 6
  /* 0Ah */ "Goldmont",                                                                 // Family 6
  /* 0Bh */ "Goldmont Plus",                                                            // Family 6
  /* 0Ch */ "Tremont",                                                                  // Family 6
  /* 0Dh */ "P6",                                                                       // Family 6
  /* 0Eh */ "Penryn (Server)",                                                          // Family 6
  /* 0Fh */ "Nehalem (Server)",                                                         // Family 6
  /* 10h */ "Westmere (Server)",                                                        // Family 6
  /* 11h */ "Sandy Bridge (Server)",                                                    // Family 6
  /* 12h */ "Ivy Bridge (Server)",                                                      // Family 6
  /* 13h */ "Haswell (Server)",                                                         // Family 6
  /* 14h */ "Broadwell (Server)",                                                       // Family 6
  /* 15h */ "Skylake (Server) / Cascade Lake / Cooper Lake",                            // Family 6
  /* 16h */ "Ice Lake (Server)",                                                        // Family 6
  /* 17h */ "Sapphire Rapids",                                                          // Family 6
  /* 18h */ "Pentium M",                                                                // Family 6
  /* 19h */ "Modified Pentium M",                                                       // Family 6
  /* 1Ah */ "Core (Client)",                                                            // Family 6
  /* 1Bh */ "Penryn (Client)",                                                          // Family 6
  /* 1Ch */ "Nhalem (Client)",                                                          // Family 6
  /* 1Dh */ "Westmere (Client)",                                                        // Family 6
  /* 1Eh */ "Sandy Bridge (Client)",                                                    // Family 6
  /* 1Fh */ "Ivy Bridge (Client)",                                                      // Family 6
  /* 20h */ "Haswell (Client)",                                                         // Family 6
  /* 21h */ "Broadwell (Client)",                                                       // Family 6
  /* 22h */ "Skylake (Client)",                                                         // Family 6
  /* 23h */ "Whiskey Lake / Amber Lake / Comet Lake / Kaby Lake / Coffee Lake",         // Family 6
  /* 24h */ "Cannon Lake",                                                              // Family 6
  /* 25h */ "Comet Lake",                                                               // Family 6
  /* 26h */ "Ice Lake (Client)",                                                        // Family 6
  /* 27h */ "Tiger Lake",                                                               // Family 6
  /* 28h */ "Rocket Lake",                                                              // Family 6
  /* 29h */ "Alder Lake",                                                               // Family 6
  /* 2Ah */ "Knights Corner",                                                           // Family 11
  /* 2Bh */ "Knights Ferry",                                                            // Family 11
  /* 2Ch */ "Netburst",                                                                 // Family 15
};


const IntelSignature intelSignatures[] = {
  // ================ Family 4 ================
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x4, 
    /* model    */ 0x1, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[1])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x4, 
    /* model    */ 0x2, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[1])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x4, 
    /* model    */ 0x3, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[1])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x4, 
    /* model    */ 0x4, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[1])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x4, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[1])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x4, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[1])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x4, 
    /* model    */ 0x8, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[1])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x4, 
    /* model    */ 0x9, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[1])
  },
// ================ Family 5 ================
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x5, 
    /* model    */ 0x1, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[2])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x5, 
    /* model    */ 0x2, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[2])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x5, 
    /* model    */ 0x4, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[2])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x5, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[2])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x5, 
    /* model    */ 0x8, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[2])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x5, 
    /* model    */ 0x9, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[3])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x5, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[3])
  },
  // ================ Family 6 ================
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x8, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[4])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x5, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[5])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xC, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[6])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x2, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[6])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x2, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[7])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x3, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[7])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x3, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[7])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x3, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[8])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x4, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[8])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x4, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xD, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[8])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x5, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[8])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x5, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xD, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[8])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x4, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xC, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[9])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x5, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xC, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[10])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x5, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xF, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[10])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x7, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[11])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x8, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[12])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x9, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[12])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x9, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xC, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[12])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[13])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[14])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xD, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[14])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[15])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[15])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x2, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[15])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x2, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xF, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[16])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x2, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xC, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[16])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x2, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xD, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[17])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x3, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[18])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x3, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xF, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[19])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x5, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[20])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x4, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xF, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[20])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x5, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[21])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x6, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[22])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x6, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xC, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[22])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x8, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xF, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[23])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x1, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[13])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x3, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[13])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[13])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[13])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[13])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x8, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[13])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xB, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[13])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x9, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[24])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xD, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[24])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[24])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[25])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xF, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[26])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[26])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[27])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[28])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x1, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xF, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[28])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x2, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[29])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x2, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[30])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x3, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[31])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x3, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xC, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[32])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x4, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[32])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x4, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[32])
  },
  IntelSignature {
    /* exFamily */ 0x0,
    /* exModel  */ 0x4, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[33])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x3, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xD, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[33])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x4, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[34])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x5, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[34])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x8, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[35])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x9, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[35])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x6, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[36])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0xA, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x5, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[37])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x7, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xD, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[38])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x7, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xE, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[38])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x8, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xC, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[39])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x8, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xD, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[39])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0xA, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[40])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x9, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0xA, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[41])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x9, 
    /* type     */ 0x0, 
    /* family   */ 0x6, 
    /* model    */ 0x7, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[41])
  },
  // ================ Family 11 ================
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0xB, 
    /* model    */ 0x1, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[42])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0xB, 
    /* model    */ 0x0, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[43])
  },
  // ================ Family 15 ================
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0xF, 
    /* model    */ 0x1, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[44])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0xF, 
    /* model    */ 0x2, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[44])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0xF, 
    /* model    */ 0x3, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[44])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0xF, 
    /* model    */ 0x4, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[44])
  },
  IntelSignature {
    /* exFamily */ 0x0, 
    /* exModel  */ 0x0, 
    /* type     */ 0x0, 
    /* family   */ 0xF, 
    /* model    */ 0x6, 
    /* stepping */ 0x0,
    /* strArch  */ &(INTEL_ARCH_LIST[44])
  }
};

void getIntelCpuInfo();

void getIntelSignature(uint8_t extendedFamily, uint8_t extendedModel, uint8_t type, uint8_t family, uint8_t model, uint8_t steppingId, IntelSignature* IntelSignature);

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
    
    const char* brandStr          = DEFAULT_STR;  // Processor brand string
    const char* processorTypeStr  = DEFAULT_STR;  // Processor type string
    const char* processorArchStr  = DEFAULT_STR;  // Processor architecture
    IntelSignature intelSignature;


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

    // Retrieve processor architecture
    getIntelSignature(extendedFamily, extendedModel, processorType, family, model, steppingId, &intelSignature);
    if (intelSignature.strArch != 0) {
      processorArchStr = (const char*) *(intelSignature.strArch);
    }

    // Retrieve processor type string
    if (processorType < 5) {
      processorTypeStr = INTEL_PROCESSOR_TYPE_LIST[processorType];
    }

    // Retrieve processor brand string
    if (eax_max >= 0x80000004) {
      // Extended Brand string is supported
      // - Brand has 48 chars long.
      // - Since each register is 4 bytes long in x86 and we read from 12 registers this gave to us (48 bytes = 4 * 12)
      uint32_t mBrandStr[12];

      // Read brand string from registers
      cpuid(0x80000002, mBrandStr[0], mBrandStr[1], mBrandStr[2], mBrandStr[3]);
      cpuid(0x80000003, mBrandStr[4], mBrandStr[5], mBrandStr[6], mBrandStr[7]);
      cpuid(0x80000004, mBrandStr[8], mBrandStr[9], mBrandStr[10], mBrandStr[11]);

      brandStr = (const char*) mBrandStr;
    } else if (brand > 0 && brand < 24) {
      // Extended Brand string unsupported but Brand id is supported
      const char* mBrandStr = INTEL_BRAND_ID_LIST[brand];

      if (brand == 0x03 && eax == 0x000006B1) {
        // If processor signature = 000006B1h, then Intel(R) Celeron(R) processor
        mBrandStr = INTEL_BRAND_ID_LIST[1];
      } else if (brand == 0x0B && eax == 0x00000F13) {
        // If processor signature = 00000F13h, then Intel(R) Xeon(R) processor MP
        mBrandStr = INTEL_BRAND_ID_LIST[12];
      } else if (brand == 0x0E && eax == 0x00000F13) {
        // If processor signature = 00000F13h, then Intel(R) Xeon(R) processor
        mBrandStr = INTEL_BRAND_ID_LIST[11];
      }

      brandStr = const_cast<char*>(mBrandStr);
    } /* else { If brand is unsupported should use processor signature in conjunction with cache descriptors to identify the processor */
    
    // Print brand string gathered from INTEL_BRAND_ID_LIST
    stdio::kprintf("CPUID - BRAND: %s - REVISION: %x", brandStr);
    stdio::kprintf(" - REVISION: %x\n", steppingId);
    stdio::kprintf("CPUID - FAMILY: %d - MODEL: %d - ARCH: %s\n", family, model, processorArchStr);
    stdio::kprintf("CPUID - CACHE: %d bytes - CORES: %d - APIC_ID: %x - TYPE: %s\n", chunks * 8, count, apicId, processorTypeStr);
}

void getIntelSignature(uint8_t extendedFamily, uint8_t extendedModel, uint8_t type, uint8_t family, uint8_t model, uint8_t steppingId, IntelSignature* intelSignature) {
  int signatureListSize = sizeof(intelSignatures) / sizeof(IntelSignature);
  int i;
  for (i=0; i<signatureListSize; i++) {
    *intelSignature = intelSignatures[i];
    if (
      intelSignature->exFamily == extendedFamily && 
      intelSignature->exModel == extendedModel &&
      intelSignature->family == family &&
      intelSignature->model == model
    ) {
      // Signature found
      break;
    } else if (i == signatureListSize - 1) {
      // End of list reached and signature not found. Reset signature
      *intelSignature = {};
    }
  }
}