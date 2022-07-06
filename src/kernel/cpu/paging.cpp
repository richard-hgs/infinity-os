// stdlibs
#include "stdio.h"
// drivers - legacy
#include "vga.h"
#include "paging.h"

// frames list control the frames that are in use and free
// Each frame is one bit
// If the bit is set the frame is in use
// If the bit is unset the frame is free
unsigned char frames[FRAMES_COUNT];
PageDirectory* pageDirectory;

void paging_test();

/**
 * @brief Returns the frame number of the next free frame
 * 
 * @return unsigned int   Next free frame number
 */
unsigned int frame_alloc();

/**
 * @brief Set the frame number (frameNr) as free to be used by some other process
 * 
 * @param frameNr The frame number being free
 */
void frame_free(unsigned int frameNr);

/**
 * @brief Given a frameNr set if the frame is in use or free
 * 
 * @param frameNr   The number of the frame being modified
 * @param usage     1=in_use, 0=free
 */
void frame_setUsage(unsigned int frameNr, int usage);

/**
 * @brief Retrieve an address in RAM memory for given frameNr
 *        Since each frame has 4096 bytes it is used to generate address multiples of 4096 bytes
 * 
 * @param frameNr           Frame number to calculate offset
 * @return unsigned int     Frame address
 */
unsigned int frame_address(unsigned int frameNr);

/**
 * @brief Retrieve a frame number given a frameAddress in RAM memory
 * 
 * @param frameAddress      Frame address in memory to calculate frame number
 * @return unsigned int     Frame number
 */
unsigned int frame_number(unsigned int frameAddress);

/**
 * @brief Retrieve frame amount for a given size of memory
 * 
 * @param size              Requested size
 * @return unsigned int     Frame count for given size
 */
unsigned int size_inFrames(unsigned int size);

void paging_enable();
void paging_disable();
void set_pageDirectory(PageDirectory *pageDirectory);
void set_pageTableEntry(PageTableEntry *tableEntry, unsigned int frameAddress, unsigned int present, unsigned int rw, unsigned int userMode);

/**
 * @brief Maps virtual address to physical address
 *        virtualAddr should be 0x1000 aligned
 * 
 * @param pageDir       The root structure that holds all PageTables and All Frames
 * @param virtualAddr   The virtual address that will be assigned a physical address
 * @param physicalAddr  The physical address that will be assigned to a virtual address
 */
void map_page(PageDirectory *pageDir, unsigned int virtualAddr, unsigned int physicalAddr);
void unmap_page(unsigned int virtualAddr);
void remote_mapPage(unsigned int virtualAddr, unsigned int physicalAddr);

/**
 * Flush page table cache.
 */
void pages_refresh();


void paging::install() {
    int i;

    for (i = 0; i < FRAMES_COUNT; i++)
        frames[i] = 0; // unused

    // frames for page directory and page tables are set as in use
    frame_setUsage(PAGE_DIRECTORY_START, 1);
    for (i = 0; i < PAGE_TABLE_COUNT; i++)
        frame_setUsage(PAGE_TABLES_START + i, 1);

    pageDirectory = (PageDirectory*) frame_address(PAGE_DIRECTORY_START);
    // all pages are set as not present
    for (i = 0; i < 1024; i++)
    {
        set_pageTableEntry(&pageDirectory->entry[i], 0, 0, 0, 0); //not present
        // PageTable* pageTable = (PageTable*) frame_address(PAGE_TABLES_START + i);
        //for (j = 0; j < 1024; j++)
            //set_pageTableEntry(&pageTable->entry[j], 0, 0, 0, 0); //not present
    }

    set_pageDirectory(pageDirectory);

    // Map kernel source code where virtual addr = physical addr
    for (i = 0; i < KERNEL_SOURCE_SIZE; i++) {
        map_page(pageDirectory, KERNEL_START_ADDR + i*FRAME_SIZE, KERNEL_START_ADDR + i*FRAME_SIZE);
    }

    // Map kernel stack where virtual addr = physical addr
    for (i = 0; i < 4; i++) {
        map_page(pageDirectory, KERNEL_STACK_START_ADDR + i*FRAME_SIZE, KERNEL_STACK_START_ADDR + i*FRAME_SIZE);
    }
    map_page(pageDirectory, VIDEO_MEM_START, 0xB8000); // mapping virtual video memory

    // mapping kernel heap where virtual addr = physical addr
    for (i = 0; i < KERNEL_HEAP_SIZE; i++)
        map_page(pageDirectory, KERNEL_HEAP_START_ADDR + i*FRAME_SIZE, KERNEL_HEAP_START_ADDR + i*FRAME_SIZE);

    // map page directory and page tables
    map_page(pageDirectory, frame_address(PAGE_DIRECTORY_START), frame_address(PAGE_DIRECTORY_START));
    for (i = 0; i < PAGE_TABLE_COUNT; i++)
        map_page(pageDirectory, frame_address(PAGE_TABLES_START + i), frame_address(PAGE_TABLES_START + i));

    // Enable paging by setting to 1 the bit 0 of cr0 register
    paging_enable();

    // Set the new vga address to the new virtual address
    vga::setVgaAddress(VIDEO_MEM_START);
}

void paging::test() {
    // Test if paging fault is ok by reading a paging not present
    // uint32_t *ptr = (uint32_t*) 0xA0000000;
    // uint32_t do_page_fault = *ptr;

    // int virtualAddr = 0x200000;
    // int pageTableNr;    // page table number inside page directory OBS
    // int pageNr;         // page number inside page table

    // pageTableNr = virtualAddr >> 22;
    // pageNr = (virtualAddr >> 12) & 1023;

    // stdio::kprintf("pageTableNr: %d\n", pageTableNr);
    // stdio::kprintf("pageNr: %d\n", pageNr);

    // ===========================================================

    // unsigned int frameNr = 7;
    // int usage = 0;

    // unsigned int byteNr; // byte number in frames buffer
    // unsigned int bitNr;
    // unsigned char mask;

    // byteNr = frameNr / 8;
    // bitNr = frameNr % 8;
    // mask = 1 << bitNr;
    // if (usage == 1)
    //     frames[byteNr] = frames[byteNr] | mask; 
    // if (usage == 0)
    //     frames[byteNr] = frames[byteNr] & ~mask;

    // stdio::kprintf("byteNr: %d\n", byteNr);
    // stdio::kprintf("bitNr: %d\n", bitNr);
    // stdio::kprintf("mask: %d\n", mask);
    // stdio::kprintf("~mask %d\n", ~mask);
}


unsigned int frame_alloc() {
    int i, j;
    unsigned char temp;     // bit number in byte
    unsigned int frameNr;   // frame number

    for (i = 0; i < FRAMES_COUNT; i++) { // For each frame
        if (frames[i] != 0xFF) {         // Check if the byte of the frame has at least one frame (bit) = 0 if so the value will be different of 0xFF (255 = 11111111)
            temp = frames[i];            // The byte has a free frame (bit) so we need to iterate the frames (bits) of the current byte to check wich frame (bit) is free

            for (j = 0; j < 8; j++) {           // For each frame in current byte
                if ((temp & 1) == 0) {          // Check if current frame (bit) of the current byte is free. Use an AND(& 1) operation in the bit if equals 0 it is free. 
                    frameNr = i * 8 + j;        // Since the frame is free we need to get this frame number. So multiply byte(i) * 8 frames(bit) + curByteFrameIndex (j)bits.
                    frame_setUsage(frameNr, 1); // Since we are allocating this frame set in_use 
                    return frameNr;             // Return this new allocated frame
                }
                temp >>= 1;                     // Shift the bits to the left of the temp(byte) by one position for each bit being verified until last bit reached or free frame found.
            }                                   // This operation is used to perform an (&)AND operation in the first bit of the current byte and verify if it is in_use.
        }
    }

    // TODO: this is wrong. Error should be returned. But currently there's
    // not way to do this.
    return 0;
}

void frame_free(unsigned int frameNr) {
    frame_setUsage(frameNr, 0);
}

void frame_setUsage(unsigned int frameNr, int usage) {
    unsigned int byteNr; // byte number location where frameNr is located in frames buffer
    unsigned int bitNr;  // bit number of the byte that is where the frame usage stored
    unsigned char mask;  // the mask that will be used to change bit value of the frame to 1(in_use) or 0(free)

    byteNr = frameNr / 8;
    bitNr = frameNr % 8;
    mask = 1 << bitNr;
    if (usage == 1) {
        frames[byteNr] = frames[byteNr] | mask;  // Perform a OR operation to set the bit to 1 and keep the others bits untouched
    }
    if (usage == 0) {
        frames[byteNr] = frames[byteNr] & ~mask;  // Perform a NOT operation in mask to inverse the value (128d = 100000000 becomes -129d = 011111111)
    }                                             // Then performs a AND operation to set the bit to zero and keep the others bits untouched
}


unsigned int frame_address(unsigned int frameNr) {
    return FRAMES_START_ADDR + frameNr * FRAME_SIZE;
}


unsigned int frame_number(unsigned int frameAddress) {
    return (frameAddress - FRAMES_START_ADDR) / FRAME_SIZE;
}


unsigned int size_inFrames(unsigned int size) {
    unsigned int frameCount;

    frameCount = size / FRAME_SIZE; // Divide the size by the frame size
    if (size % FRAME_SIZE > 0) {    // If there is rest the frame amount don't fit requested size.
        frameCount++;               // So we increment one more frame.
    }
    return frameCount;
}

void set_pageTableEntry(PageTableEntry *tableEntry, unsigned int frameAddress, unsigned int present, unsigned int rw, unsigned int userMode)
{
    tableEntry->frameAddress = frameAddress;
    tableEntry->present = present;
    tableEntry->rw = rw;
    tableEntry->userMode = userMode;

    tableEntry->dirty = 0;
    tableEntry->reserved1 = 0;
    tableEntry->reserved2 = 0;
    tableEntry->accessed = 0;
    tableEntry->unused = 0;
}

void paging_enable()
{
    asm volatile ("mov %%cr0, %%eax;"
        "or $0x80000000, %%eax;"
        "mov %%eax, %%cr0"
        : : : "eax"
       );
}

void paging_disable()
{
    asm volatile ("mov %%cr0, %%eax;"
        "and $0x7FFFFFFF, %%eax;"
        "mov %%eax, %%cr0"
        : : : "eax"
       );
}

void set_pageDirectory(PageDirectory* pageDirectory)
{
    asm volatile ("mov %0, %%eax;"
        "mov %%eax, %%cr3"
        : : "r" (pageDirectory)
        : "eax"
        );
}

/**
 * @brief Maps virtual address to physical address
 *        virtualAddr should be 0x1000 aligned
 * 
 * @param pageDir       The root structure that holds all PageTables and All Frames
 * @param virtualAddr   The virtual address that will be assigned a physical address
 * @param physicalAddr  The physical address that will be assigned to a virtual address
 */
void map_page(PageDirectory* pageDir, unsigned int virtualAddr, unsigned int physicalAddr)
{
    int pageTableNr;        // Page table number inside page directory (The main directory holds 1024 PageTable)
    int pageNr;             // Page number inside page table (A PageTable holds 1024 PageEntries)
    PageTable *pageTable;

    //                                                    ______________________________
    // The virtual address is a 32 bits splitted between | 31---22 | 21------12 | 11--0 |
    //                                                   |PAGE_DIR | PAGE_TABLE | OFFSET|
    //                                                    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    // PAGE_DIR (Has 1024 pageTableNr)   - Bits: 31-22 = 10 bits = 1024 addresses
    // PAGE_TABLE (Has 1024 pageNr)      - Bits: 21-12 = 10 bits = 1024 addresses
    // OFFSET     (Has 4096 frame size)  - Bits: 0-11  = 12 bits = 4096 addresses
    pageTableNr = virtualAddr >> 22;            // Since the OFFSET + PAGE_TABLE = 22 bits. Shift those bits right to extract pageTableNr.
    pageNr = (virtualAddr >> 12) & 1023;        // Since the OFFSET = 12 bits. Shift those bits right to extract pageNr.
    pageTable = (PageTable*) frame_address(PAGE_TABLES_START + pageTableNr);
    set_pageTableEntry(&pageDir->entry[pageTableNr], (int) pageTable >> 12, 1, 1, 0);
    set_pageTableEntry(&pageTable->entry[pageNr], physicalAddr >> 12, 1, 1, 0);

    frame_setUsage(frame_number(physicalAddr), 1);
}

void unmap_page(unsigned int virtualAddr)
{
    int pageTableNr;
    int pageNr; //page number inside page table
    PageTable *pageTable;

    pageTableNr = virtualAddr >> 22;
    pageNr = (virtualAddr >> 12) & 1023;

    pageTable = (PageTable*) frame_address(PAGE_TABLES_START + pageTableNr);
    set_pageTableEntry(&pageTable->entry[pageNr], 0, 0, 0, 0);
}

void remote_mapPage(unsigned int virtualAddr, unsigned int physicalAddr)
{
    map_page(pageDirectory, virtualAddr, physicalAddr);
}

void pages_refresh()
{
    set_pageDirectory(pageDirectory);
}