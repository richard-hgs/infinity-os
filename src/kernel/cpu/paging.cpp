// stdlibs
#include "stdio.h"
// memory
#include "memutils.h"
#include "kheap.h"
#include "paging.h"

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % ( 8 * 4))

extern uint32_t __MAX_ADDR; // Max memory address that is used by kernel variables. Created in link.ld

// A bitset of frames - used or free.
uint32_t *frames;
uint32_t nframes;

// The kernel's page directory
PageDirectory_t* kernel_directory = 0;

// The current page directory;
PageDirectory_t* current_directory = 0;

// Static function to set a bit in the frames bitset
void set_frame(uint32_t frame_addr) {
   uint32_t frame = frame_addr / 0x1000;
   uint32_t idx = INDEX_FROM_BIT(frame);
   uint32_t off = OFFSET_FROM_BIT(frame);
   frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
void clear_frame(uint32_t frame_addr) {
   uint32_t frame = frame_addr / 0x1000;
   uint32_t idx = INDEX_FROM_BIT(frame);
   uint32_t off = OFFSET_FROM_BIT(frame);
   frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
uint32_t test_frame(uint32_t frame_addr) {
   uint32_t frame = frame_addr / 0x1000;
   uint32_t idx = INDEX_FROM_BIT(frame);
   uint32_t off = OFFSET_FROM_BIT(frame);
   return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
uint32_t first_frame() {
   uint32_t i, j;
   for (i = 0; i < INDEX_FROM_BIT(nframes); i++) {
       if (frames[i] != 0xFFFFFFFF) { // nothing free, exit early.
           // at least one bit is free here.
           for (j = 0; j < 32; j++) {
               uint32_t toTest = 0x1 << j;
               if (!(frames[i]&toTest)) {
                   return i*4*8+j;
               }
           }
       }
   }
   return -1; // If no free frames found return error
}

// Function to allocate a frame.
void alloc_frame(PageTableEntry_t* page, int is_kernel, int is_writeable) {
   if (page->baseAddress != 0) {
       return; // Frame was already allocated, return straight away.
   } else {
       uint32_t idx = first_frame(); // idx is now the index of the first free frame.
       if (idx == (uint32_t)-1) {
           // PANIC is just a macro that prints a message to the screen then hits an infinite loop.
           PANIC("No free frames!");
       }
       set_frame(idx * 0x1000);                  // this frame is now ours!
       page->present = 1;                        // Mark it as present.
       page->readWrite = (is_writeable) ? 1 : 0; // Should the page be writeable?
       page->supervisor = (is_kernel) ? 0 : 1;   // Should the page be user-mode?
       page->baseAddress = idx;
   }
}

// Function to deallocate a frame.
void free_frame(PageTableEntry_t* page) {
   uint32_t frame;
   if (!(frame=page->baseAddress)) {
       return; // The given page didn't actually have an allocated frame!
   } else {
       clear_frame(frame);      // Frame is now free again.
       page->baseAddress = 0x0; // Page now doesn't have a frame.
   }
}

PageTableEntry_t* get_page(uint32_t address, int make, PageDirectory_t* dir) {
   // Turn the address into an index.
   address /= 0x1000;
   // Find the page table containing this address.
   uint32_t table_idx = address / 1024;
   if (dir->tables[table_idx]) { // If this table is already assigned
       return &dir->tables[table_idx]->pages[address % 1024];
   } else if(make) {
       uint32_t tmp;
       dir->tables[table_idx] = (PageTable_t*) kheap::kmalloc_ap(sizeof(PageTable_t), &tmp);
       memutils::memset(dir->tables[table_idx], 0, 0x1000);
       dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
       return &dir->tables[table_idx]->pages[address % 1024];
   } else {
       return 0;
   }
}

void switch_page_directory(PageDirectory_t* dir) {
   current_directory = dir;
    asm volatile ("mov %0, %%eax;"
        "mov %%eax, %%cr3"
        : : "r" (&dir->tablesPhysical)
        : "eax"
    );
    asm volatile ("mov %%cr0, %%eax;"
        "or $0x80000000, %%eax;"
        "mov %%eax, %%cr0"
        : : : "eax"
    );


   // asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
//    uint32_t cr0;
//    asm volatile("mov %%cr0, %0": "=r"(cr0));
//    cr0 |= 0x80000000; // Enable paging!
//    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

void paging::install() {
    uint32_t placement_address = (uint32_t) &__MAX_ADDR; // Get the pointer address of this variable wich is the last address of the kernel in memory;

    nframes = PAGE_MEM_ADDRESS_END / 0x1000;
    frames = (uint32_t*) kheap::kmalloc(INDEX_FROM_BIT(nframes));
    memutils::memset(frames, 0, INDEX_FROM_BIT(nframes));

    // Let's make a page directory.
    kernel_directory = (PageDirectory_t*) kheap::kmalloc_a(sizeof(PageDirectory_t));
    memutils::memset(kernel_directory, 0, sizeof(PageDirectory_t));
    current_directory = kernel_directory;

    // We need to identity map (phys addr = virt addr) from
    // 0x0 to the end of used memory, so we can access this
    // transparently, as if paging wasn't enabled.
    // NOTE that we use a while loop here deliberately.
    // inside the loop body we actually change placement_address
    // by calling kmalloc(). A while loop causes this to be
    // computed on-the-fly rather than once at the start.
    uint32_t i = 0;
    while (i < placement_address) {
        // Kernel code is readable but not writeable from userspace.
        alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }

    // stdio::kprintf("pageEntrySize: %x", placement_address);
    // while(true) {}

    // Now, enable paging!
    switch_page_directory(kernel_directory);
    
    while(true) {}

    // stdio::kprintf("pageDirectory: %x\n", (uint32_t) kernel_directory);
}