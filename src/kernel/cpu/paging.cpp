// stdlibs
#include "stdio.h"
// drivers - legacy
#include "paging.h"
#include "vga.h"

// frames list control the frames that are in use and free
// Each frame is one bit
// If the bit is set the frame is in use
// If the bit is unset the frame is free
unsigned char frames[FRAMES_COUNT];

/**
 * @brief Pointer to the start of kernel Paging entry structure 
 * that maps the kernel source, stack, heap...
 * 
 */
PageDirectory* pageDirectory;

// ====================================================================================================

void paging::install() {
    int i;

    // Initialize the frames to 0=UNUSED, because frames is located in .bss section and must be initialized.
    for (i = 0; i < FRAMES_COUNT; i++) { 
        frames[i] = 0; // unused
    }

    // Frames for page directory and page tables are set as in use
    frameSetUsage(PAGE_DIRECTORY_START, 1);
    for (i=0; i<PAGE_TABLE_COUNT; i++) {
        frameSetUsage(PAGE_TABLES_START + i, 1);
    }

    // PageDirectory is located in .bss section and must be initialized.
    // PageDirectory from (0x100000 - 0x101000) = 0x1000 = 4kb
    pageDirectory = (PageDirectory*) frameAddress(PAGE_DIRECTORY_START);

    // All pages are set as not present
    for (i=0; i<1024; i++) {
        // Set all directory entries as not present in memory
        // Wich means that the page doesn't have a physical address
        setPageTableEntry(&pageDirectory->entry[i], 0, 0, 0, 0); // not present
    }

    // Set the page directory pointer in cr3 register
    setPageDirectory(pageDirectory);

    // Map kernel source code where virtual addr = physical addr
    // from (0x6400000 - 0x6500000) = 0x100000 = 1Mb
    for (i=0; i<KERNEL_SOURCE_SIZE; i++) {
        mapPage(pageDirectory, KERNEL_START_ADDR + i * FRAME_SIZE, KERNEL_START_ADDR + i * FRAME_SIZE);
    }

    // Map kernel stack where virtual addr = physical addr
    // from (0x6501000 - 0x6505000) = 0x4000 = 16kb
    for (i=0; i<4; i++) {
        mapPage(pageDirectory, KERNEL_STACK_START_ADDR + i * FRAME_SIZE, KERNEL_STACK_START_ADDR + i * FRAME_SIZE);
    }

    // Mapping virtual video memory
    // from (0x6506000 - 0x6507000) = 0x1000 = 4kb
    mapPage(pageDirectory, VIDEO_MEM_START, 0xB8000); 

    // Mapping kernel heap where virtual addr = physical addr
    // from (0x6507000 - 0x7107000) = 0xC00000 = 12 Mb
    for (i=0; i<KERNEL_HEAP_SIZE; i++) {
        mapPage(pageDirectory, KERNEL_HEAP_START_ADDR + i * FRAME_SIZE, KERNEL_HEAP_START_ADDR + i * FRAME_SIZE);
    }

    // Map page directory and page tables
    // from (0x100000 - 0x101000) = 0x1000 = 4kb
    mapPage(pageDirectory, frameAddress(PAGE_DIRECTORY_START), frameAddress(PAGE_DIRECTORY_START));
    for (i=0; i<PAGE_TABLE_COUNT; i++) {
        // from (0x101000 - 0x500000) = 0x3FF000 = 4 Mb
        mapPage(pageDirectory, frameAddress(PAGE_TABLES_START + i), frameAddress(PAGE_TABLES_START + i));
    }

    // Enable paging by setting to 1 the bit 31 of cr0 register
    pagingEnable();

    // Set the new vga address to the new virtual address
    vga::setVgaAddress(VIDEO_MEM_START);

    // stdio::kprintf("");

    // __asm__ volatile ("cli; hlt");  // Halt the cpu Completely hangs the computer
}

void paging::test() {
    // Test if paging fault is ok by reading a paging not present
    // uint32_t *ptr = (uint32_t*) 0xA0000000;
    // uint32_t do_page_fault = *ptr;

    // ==========================================================

    // int virtualAddr = 0x300000;
    // int pageTableNr;    // page table number inside page directory OBS
    // int pageNr;         // page number inside page table

    // pageTableNr = virtualAddr >> 22;
    // pageNr = (virtualAddr >> 12) & 1023;
    // PageTable* pageTable = (PageTable*) frameAddress(PAGE_TABLES_START + pageTableNr);

    // stdio::kprintf("pageTableNr: %d\n", pageTableNr);
    // stdio::kprintf("pageNr: %d\n", pageNr);
    // stdio::kprintf("pageTable: %x\n", (uint32_t) pageTable);

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

unsigned int paging::frameAlloc() {
    int i, j;
    unsigned char temp;   // bit number in byte
    unsigned int frameNr; // frame number

    for (i=0; i<FRAMES_COUNT; i++) { // For each frame
        if (frames[i] != 0xFF) {         // Check if the byte of the frame has at least one frame (bit) = 0 if so the value will be different of 0xFF (255 = 11111111)
            temp = frames[i];            // The byte has a free frame (bit) so we need to iterate the frames (bits) of the current byte to check wich frame (bit) is free

            for (j = 0; j < 8; j++) {           // For each frame in current byte
                if ((temp & 1) == 0) {          // Check if current frame (bit) of the current byte is free. Use an AND(& 1) operation in the bit if equals 0 it is free.
                    frameNr = i * 8 + j;        // Since the frame is free we need to get this frame number. So multiply byte(i) * 8 frames(bit) + curByteFrameIndex (j)bits.
                    frameSetUsage(frameNr, 1); // Since we are allocating this frame set in_use
                    return frameNr;             // Return this new allocated frame
                }
                temp >>= 1; // Shift the bits to the left of the temp(byte) by one position for each bit being verified until last bit reached or free frame found.
            }               // This operation is used to perform an (&)AND operation in the first bit of the current byte and verify if it is in_use.
        }
    }

    // TODO: this is wrong. Error should be returned. But currently there's
    // not way to do this.
    return 0;
}

void paging::frameFree(unsigned int frameNr) {
    frameSetUsage(frameNr, 0);
}

void paging::frameSetUsage(unsigned int frameNr, int usage) {
    unsigned int byteNr; // byte number location where frameNr is located in frames buffer
    unsigned int bitNr;  // bit number of the byte that is where the frame usage stored
    unsigned char mask;  // the mask that will be used to change bit value of the frame to 1(in_use) or 0(free)

    byteNr = frameNr / 8;
    bitNr = frameNr % 8;
    mask = 1 << bitNr;
    if (usage == 1) {
        frames[byteNr] = frames[byteNr] | mask; // Perform a OR operation to set the bit to 1 and keep the others bits untouched
    }
    if (usage == 0) {
        frames[byteNr] = frames[byteNr] & ~mask; // Perform a NOT operation in mask to inverse the value (128d = 100000000 becomes -129d = 011111111)
    }                                            // Then performs a AND operation to set the bit to zero and keep the others bits untouched
}

unsigned int paging::frameAddress(unsigned int frameNr) {
    return FRAMES_START_ADDR + frameNr * FRAME_SIZE;
}

unsigned int paging::frameAddress(PageDirectory* pageDir, unsigned int pageTableNr) {
    return (int) pageDir | pageDir->entry[pageTableNr].frameAddress << 12;
}

unsigned int paging::framePhysicalAddress(PageDirectory* pageDir, unsigned int pageTableNr, unsigned int pageNr, unsigned int virtualAddress) {
    PageTable* pageTable = (PageTable*) frameAddress(pageDir, pageTableNr);
    return pageDir->entry[pageTableNr].frameAddress << 22 | pageTable->entry[pageNr].frameAddress << 12 | (virtualAddress & 0xFFF); // The low 12 bits remaining are the same from the virtual address
}

unsigned int paging::frameNumber(unsigned int frameAddress) {
    return (frameAddress - FRAMES_START_ADDR) / FRAME_SIZE;
}

unsigned int paging::sizeInFrames(unsigned int size) {
    unsigned int frameCount;

    frameCount = size / FRAME_SIZE; // Divide the size by the frame size
    if (size % FRAME_SIZE > 0) {    // If there is rest the frame amount don't fit requested size.
        frameCount++;               // So we increment one more frame.
    }
    return frameCount;
}

void paging::setPageTableEntry(PageTableEntry* tableEntry, unsigned int frameAddress, unsigned int present, unsigned int rw, unsigned int userMode) {
    tableEntry->frameAddress = frameAddress;  // Physical page address offset (in a DIRECTORY the PAGE_TABLE address is right shifted by 12 bits);
    tableEntry->present      = present;       // 0=Page isn't in RAM,               1=Page is in RAM;
    tableEntry->rw           = rw;            // 0=Read Only,                       1=Read/Write;
    tableEntry->userMode     = userMode;      // 0=Supervisor privilege,            1=User privilege

    tableEntry->dirty        = 0;             // 0=No changes,                      1=Page changed and need to be updated in secondary memory
    tableEntry->reserved1    = 0;
    tableEntry->reserved2    = 0;
    tableEntry->accessed     = 0;             // 0=No access performed by CPU       1=Read or Write in this page performed by the cpu
    tableEntry->unused       = 0;
}

void paging::pagingEnable() {
    // Set the 31º bit = 1 to enable the paging in MMU Memory Management Unity
    // 0x80000000 = 10000000000000000000000000000000
    // Perform an "OR" operation in cr0 register to set the bit and keep others untouched
    asm volatile("mov %%cr0, %%eax;"
                 "or $0x80000000, %%eax;"
                 "mov %%eax, %%cr0"
                 : /*output*/
                 : /*input*/
                 : /*clobbers*/ "eax");
}

void paging::pagingDisable() {
    // Set the 31º bit = 0 to disable the paging in MMU Memory Management Unity
    // 0x7FFFFFFF = 01111111111111111111111111111111
    // Perform a "AND" operation in cr0 register to set the bit and keep others untouched
    asm volatile("mov %%cr0, %%eax;"
                 "and $0x7FFFFFFF, %%eax;"
                 "mov %%eax, %%cr0"
                 : /*output*/
                 : /*input*/
                 : /*clobbers*/ "eax");
}

void paging::setPageDirectory(PageDirectory* pageDirectory) {
    // Set the pointer reference of pageDirectory in cr3 register
    // The CR3 register, tells the CPU where the page table is in RAM memory
    // The page table is used by the CPU to perform memory checks and protections mechanism, such as access and others...
    asm volatile("mov %0, %%eax;"
                 "mov %%eax, %%cr3"
                 :                              /*output*/
                 : /*input*/ "r"(pageDirectory) // r=General Purpose Registers
                 : /*clobbers*/ "eax");
}

/**
 * @brief Maps virtual address to physical address
 *        virtualAddr should be 0x1000 aligned
 *
 * @param pageDir       The root structure that holds all PageTables and All Frames
 * @param virtualAddr   The virtual address that will be assigned a physical address
 * @param physicalAddr  The physical address that will be assigned to a virtual address
 */
void paging::mapPage(PageDirectory* pageDir, unsigned int virtualAddr, unsigned int physicalAddr) {
    int pageTableNr;       // Page table number inside page directory (The main directory holds 1024 PageTable)
    int pageNr;            // Page number inside page table (A PageTable holds 1024 PageEntries)
    PageTable* pageTable;

    //                                                    ______________________________
    // The virtual address is a 32 bits splitted between | 31---22 | 21------12 | 11--0 |
    //                                                   |PAGE_DIR | PAGE_TABLE | OFFSET|
    //                                                    ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
    // PAGE_DIR (Has 1024 pageTableNr)   - Bits: 31-22 = 10 bits = 1024 addresses
    // PAGE_TABLE (Has 1024 pageNr)      - Bits: 21-12 = 10 bits = 1024 addresses
    // OFFSET     (Has 4096 frame size)  - Bits: 0-11  = 12 bits = 4096 addresses
    pageTableNr = virtualAddr >> 22;     // Since the OFFSET + PAGE_TABLE = 22 bits. Shift those bits right to extract pageTableNr (PAGE_DIR entry).
    pageNr = (virtualAddr >> 12) & 1023; // Since the OFFSET = 12 bits. Shift those bits right to extract pageNr (PAGE_TABLE entry).
    pageTable = (PageTable*) frameAddress(PAGE_TABLES_START + pageTableNr); // Retrieve the location of this page in physical RAM memory and convert to PageTable

    // int mPageTable = (int) pageTable >> 12;
    // int mPageTableAddr = (int) pageDir | mPageTable << 12;
    
    setPageTableEntry(&pageDir->entry[pageTableNr], (int) pageTable >> 12, 1, 1, 0);
    setPageTableEntry(&pageTable->entry[pageNr], physicalAddr >> 12, 1, 1, 0);

    frameSetUsage(frameNumber(physicalAddr), 1);

    // unsigned int framePhysicalAddress = framePhysicalAddress(pageDir, pageTableNr, pageNr, 1);

    // stdio::kprintf("pageTableNr: %d - pageNr: %d - virtualAddress: %x - physicalAddress: %x - physicalAddress2: %x\n", pageTableNr, pageNr, virtualAddr, physicalAddr, framePhysicalAddress);

    // __asm__ volatile ("cli; hlt");  // Halt the cpu Completely hangs the computer
}

void paging::unmapPage(unsigned int virtualAddr) {
    int pageTableNr;        // Page table number inside page directory (The main directory holds 1024 PageTable)
    int pageNr;             // Page number inside page table (A PageTable holds 1024 PageEntries)
    PageTable* pageTable;

    pageTableNr = virtualAddr >> 22;
    pageNr = (virtualAddr >> 12) & 1023;

    pageTable = (PageTable*) frameAddress(PAGE_TABLES_START + pageTableNr);
    setPageTableEntry(&pageTable->entry[pageNr], 0, 0, 0, 0);
}

void paging::remoteMapPage(unsigned int virtualAddr, unsigned int physicalAddr) {
    mapPage(pageDirectory, virtualAddr, physicalAddr);
}

void paging::pagesRefresh() {
    setPageDirectory(pageDirectory);
}