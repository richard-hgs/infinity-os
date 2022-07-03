// stdlibs
#include "stdio.h"
#include "paging.h"

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % ( 8 * 4))

// A bitset of frames - used or free.
uint32_t *frames;
uint32_t nframes;

// Static function to set a bit in the frames bitset
void set_frame(uint32_t frame_addr) {
   uint32_t frame = frame_addr/0x1000;
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

void paging::install() {

    // stdio::kprintf("pageDirectory: %d", pageDirectory->base);
}