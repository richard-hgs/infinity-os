// cpu
#include "cpuid.h"
#include "apic.h"

int apic::install() {
    if (cpuid::hasApic()) {
        // Apic is present and can be used
        
    } else {
        return APIC_ERROR_NOT_PRESENT;
    }
    return APIC_NO_ERROR;
}