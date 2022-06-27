#include "gdt.h"

struct gdt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

typedef struct gdt_ptr gdt_ptr_t;

gdt_entry_t gdt_entries[MAX_GDT_ENTRIES];
gdt_ptr_t gdt_ptr;

/**
 * @brief Set GDT entry parameters. Since they aren't linear we need to perform bitwise operations in 32, 16 and 8 bits variables
 * to achieve a GDT table.
 * 
 * @param i 		Entry index position
 * @param base 		The base address of the entry
 * @param limit 	The length in memory of the entry
 * @param access 	The access flags of the entry
 * @param gran 		The size configuration of the entry
 */
void gdt_set_gate(uint32_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	if(i >= MAX_GDT_ENTRIES) {
		return;
	}

	// Format the 32 bits value into the low, mid and high bits of the GDT entry since they are spread all over the table
	gdt_entries[i].base_low = (base & 0xffff);		// If base < 0xffff then base else 0xffff
	gdt_entries[i].base_mid = (base >> 16) & 0xff;	// If base*+16 < 0xff then base else 0xff
	gdt_entries[i].base_high = (base >> 24) & 0xff;	// If base*+24 < 0xff the base else 0xff

	gdt_entries[i].limit = (limit & 0xffff);		// If limit < 0xffff then limit else 0xffff

	gdt_entries[i].access = access;
 
	gdt_entries[i].grand = (limit >> 16) & 0x0f;	// If limit*+16 < 0x0f then limit else 0x0f; Saves the high 4 bits of the limit_high.
	gdt_entries[i].grand |= gran & 0xf0;			// Discards the low bits of gran since it is used by the segment limit. This is performed by (gran & 0xf0 = 11110000)
}


void gdt::install(void)
{
	gdt_ptr.limit = sizeof(gdt_entry_t) * MAX_GDT_ENTRIES - 1;
	gdt_ptr.base = (uint32_t) &gdt_entries;

	gdt_set_gate(0, 0, 0, 0, 0); 						// Null Descriptor - Required
	gdt_set_gate(1, 0, 0xffffffff, 0x9a, 0xcf); 		// Kernel - Code segment 		- (0x9a = 10011010) (0xcf = 11001111)
	gdt_set_gate(2, 0, 0xffffffff, 0x92, 0xcf); 		// Kernel - Data segment 		- (0x9b = 10010010) (0xcf = 11001111)
	gdt_set_gate(3, 0, 0xffffffff, 0xfa, 0xcf); 		// User   - Code segment 		- (0xfa = 11111010) (0xcf = 11001111)
	gdt_set_gate(4, 0, 0xffffffff, 0xf2, 0xcf); 		// User   - Data segment 		- (0xf2 = 11110010) (0xcf = 11001111)
	// gdt_set_gate(5, &tss, sizeof(tss), 0xf2, 0xcf); 	// TSS    - Task state segment

	gdt_flush((uint32_t) &gdt_ptr);
}