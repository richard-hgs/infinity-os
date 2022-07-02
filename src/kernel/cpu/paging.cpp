#include "stdio.h"
#include "paging.h"

/*
    max user memory is 1 MB because virtual addr 0x100000 is where
    page directory and page tables start
*/

unsigned char frames[FRAMES_COUNT];
PageDirectory_t* pageDirectory;

void frame_setUsage(unsigned int frameNr, int usage)
{
    unsigned int byteNr; // byte number in frames buffer
    unsigned int bitNr;
    unsigned char mask;

    byteNr = frameNr / 8;
    bitNr = frameNr % 8;
    mask = 1 << bitNr;
    if (usage == 1)
        frames[byteNr] = frames[byteNr] | mask;
    if (usage == 0)
        frames[byteNr] = frames[byteNr] & ~mask;
}

// sets frame usage bit to 0
void frame_free(unsigned int frameNr)
{
    frame_setUsage(frameNr, 0);
}

void paging::install() {
    // stdio::kprintf("pageSize: %d\n", sizeof(PageTableEntry_t));
    // pagedir_ptr->access = ; // [5=, 4=Set 0 Page has no access yet, 3=Set 1 caching enabled, 2=Set 1 Allow caching, 1=Set 1 Super privilege, 0= Set 1 R/W]

    int i;
    for (i = 0; i < FRAMES_COUNT; i++) {
        frames[i] = 0; // unused
    }


}