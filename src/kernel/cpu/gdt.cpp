
#include "gdt.h"

struct gdt_ptr {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));

typedef struct gdt_ptr gdt_ptr_t;

gdt_entry_t gdt_entries[MAX_GDT_ENTRIES];
gdt_ptr_t gdt_ptr;

/* Set a GDT entry.
 */
void gdt_set_gate(uint32_t i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
	if(i >= MAX_GDT_ENTRIES) return;

	gdt_entries[i].base_low = (base & 0xffff);
	gdt_entries[i].base_mid = (base >> 16) & 0xff;
	gdt_entries[i].base_high = (base >> 24) & 0xff;

	gdt_entries[i].limit = (limit & 0xffff);
	gdt_entries[i].grand = (limit >> 16) & 0x0f;
	gdt_entries[i].grand |= gran & 0xf0;
	gdt_entries[i].access = access;
}
/* Install new GDT into system.
 */
void gdt_install(void)
{
	gdt_ptr.limit = sizeof(gdt_entry_t) * MAX_GDT_ENTRIES - 1;
	gdt_ptr.base = (uint32_t) &gdt_entries;

	gdt_set_gate(0, 0, 0, 0, 0);
	gdt_set_gate(1, 0, 0xffffffff, 0x9a, 0xcf); // Code segment - Kernel
	gdt_set_gate(2, 0, 0xffffffff, 0x92, 0xcf); // Data segment - Kernel
	gdt_set_gate(3, 0, 0xffffffff, 0xfa, 0xcf); // Code segment - User
	gdt_set_gate(4, 0, 0xffffffff, 0xf2, 0xcf); // Data segment - User

	gdt_flush((uint32_t) &gdt_ptr);
}