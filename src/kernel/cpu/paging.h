#pragma once
#ifndef _PAGING_H_
#define _PAGING_H_

#include <stdint.h>

/**
 * @brief MEMORY MAP
 * 
 * - RAM HARDWARE MAP
 *  ______________________________________________________________________________________________________________________
 * |    START   |    END     |         SIZE        | DESCRIPTION                                                          |
 * | 0x00000000 | 0x000003FF | 1 KiB               | Real Mode IVT (Interrupt Vector Table)                               |
 * | 0x00000400 | 0x000004FF | 256 bytes           | BDA (BIOS data area)                                                 |
 * | 0x00000500 | 0x00007BFF | almost 30 KiB       | Conventional memory                                                  |
 * | 0x00007C00 | 0x00007DFF | 512 bytes           | Your OS BootSector                                                   |
 * | 0x00007E00 | 0x0007FFFF | 480.5 KiB           | Conventional memory                                                  |
 * | 0x00080000 | 0x0009FFFF | 128 KiB             | EBDA (Extended BIOS Data Area)                                       |
 * | 0x000A0000 | 0x000BFFFF | 128 KiB             | Video display memory                                                 |
 * | 0x000C0000 | 0x000C7FFF | 32 KiB (typically)  | Video BIOS                                                           |
 * | 0x000C8000 | 0x000EFFFF | 160 KiB (typically) | BIOS Expansions                                                      |
 * | 0x000F0000 | 0x000FFFFF | 64 KiB              | Motherboard BIOS                                                     |
 * | 0x00100000 | 0x00EFFFFF | 14 Mb               | RAM -- free for use (if it exists)                                   |
 * | 0x00F00000 | 0x00FFFFFF | 1 Mb                | Possible memory mapped hardware                                      |
 * | 0x01000000 | ????????   | ??(whatever exists) | RAM -- free for use                                                  |
 * | 0xC0000000 | 0xFFFFFFFF | 1 GiB               | Memory mapped PCI devices, PnP NVRAM?, IO APIC/s, local APIC/s, BIOS |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 * 
 * - KERNEL MEMORY MAP
 *  ______________________________________________________________________________________________________________________
 * |    START   |     END     |        SIZE        | DESCRIPTION                                                          |
 * | 0x0100000  | 0x0101000   | 0x001000 ( 4 Kb)   | O.S. Page Directory 1024 dir entries                                 |
 * | 0x0101000  | 0x0500000   | 0x3FF000 ( 4 Mb)   | O.S. Page Table 1024 entries                                         |
 * | 0x6400000  | 0x6500000   | 0x100000 ( 1 Mb)   | O.S. Kernel source memory                                            |
 * | 0x6501000  | 0x6505000   | 0x004000 (16 kb)   | O.S. Kernel stack memory                                             |
 * | 0x6506000  | 0x6507000   | 0x001000 ( 4 kb)   | O.S. VGA (0xB8000) video memory                                      |
 * | 0x6507000  | 0x7107000   | 0xC00000 (12 Mb)   | O.S. Kernel heap memory                                              |
 * | 
 */

#define FRAMES_COUNT 1024 * 128
#define FRAMES_START_ADDR 0x100000
#define FRAME_SIZE 4096 // 4 kB

// all numbers are in frames
#define PAGE_DIRECTORY_START 0
#define PAGE_TABLES_START 1
#define PAGE_TABLE_COUNT 1024

#define USER_PAGES_START PAGE_TABLES_START + PAGE_TABLE_COUNT // frame number where user pages start
#define BOOT_START_ADDR 0x7C00      // 31 KB
#define KERNEL_START_ADDR 0x6400000 // 100 MB
#define KERNEL_SOURCE_SIZE 256 // 1 MB = 256 frames
#define KERNEL_STACK_START_ADDR KERNEL_START_ADDR + (KERNEL_SOURCE_SIZE * FRAME_SIZE) + FRAME_SIZE // kernel + 1MB + 4kB
#define KERNEL_STACK_SIZE 4  // 16 kb = 4 frames

#define VIDEO_MEM_START KERNEL_STACK_START_ADDR + (KERNEL_STACK_SIZE + 1) * FRAME_SIZE // kernel stack + kernel stack size + 4kB

#define KERNEL_HEAP_START_ADDR VIDEO_MEM_START + FRAME_SIZE // video mem + 4kB
#define KERNEL_HEAP_SIZE 1024 * 3 // 1024 * 3 frames = 12 MB

/**
 * @brief MMU - Memory Management Unity - Paging
 *        docs/intel_x86_x64_specification.pdf page 119
 * 
 *  EXAMPLE OF THE PAGING SPECIFICATION FOR 2 LEVEL PAGING 4K FRAME SIZE
 *  PAGE_DIRECTORY 
 *    - Starts in RAM at (0x100000)
 *    - Has 1024 * 4 byte per entry = (0x1000 = 4096 bytes = 4kb)
 *   _____________________ 
 *  | PAGE_TBL_ENTRY 1    |-------------------------------------------------------------]
 *  |---------------------|                                                             |
 *  | PAGE_TBL_ENTRY 2    |------------------------------------------------------------------]
 *  |---------------------|                                                             |    |
 *  | PAGE_TBL_ENTRY 3    |                                                             |    |
 *  |---------------------|                                                             |    |
 *  | PAGE_TBL_ENTRY N    |                                                             |    |
 *  |---------------------|                                                             |    |
 *  | PAGE_TBL_ENTRY 1024 |                                                             |    |
 *   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾                                                              |    |
 *                                                                                      |    |
 *  PAGE_TBL_ENTRY_1                                                                    |    |
 *   - Starts in RAM at (0x100000 + 0x1000 = 0x101000)                                  |    |
 *   - Has 1024 * 4 byte per entry = (0x1000 = 4096 bytes = 4kb)                        |    |
 *  __________________________________________________________________________________  |    |
 * | 31 ------------------ 12 | 11 - 9 | 8 | 7  |  6  | 5 |  4  |  3  |  2  |  1  | 0 | |    |
 * | Page-Table Base Address  | Availa | G | PS | AVL | A | PCD | PWT | U/S | R/W | P | |    |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾  |    |
 *   __________________                                                                 |    |
 *  | FRAME_ENTRY 1    |----------------------------------------------------------------]    |
 *  |------------------|                                                                     |
 *  | FRAME_ENTRY 2    |---------------------------------------------------------------------------]
 *  |------------------|                                                                     |     |
 *  | FRAME_ENTRY 3    |                                                                     |     |
 *  |------------------|                                                                     |     |
 *  | FRAME_ENTRY N    |                                                                     |     |
 *  |------------------|                                                                     |     |
 *  | FRAME_ENTRY 1024 |                                                                     |     |
 *   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾                                                                      |     |
 *                                                                                           |     |
 *  PAGE_TBL_ENTRY_2                                                                         |     |
 *   - Starts in RAM at (0x101000 + 0x1000 = 0x102000)                                       |     |
 *   - Has 1024 * 4 byte per entry = (0x1000 = 4096 bytes = 4kb)                             |     |
 *  __________________________________________________________________________________       |     |
 * | 31 ------------------ 12 | 11 - 9 | 8 | 7  |  6  | 5 |  4  |  3  |  2  |  1  | 0 |      |     |
 * | Page-Table Base Address  | Availa | G | PS | AVL | A | PCD | PWT | U/S | R/W | P |      |     |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾       |     |
 *   __________________                                                                      |     |
 *  | FRAME_ENTRY 1    |---------------------------------------------------------------------]     |
 *  |------------------|                                                                           |
 *  | FRAME_ENTRY 2    |                                                                           |
 *  |------------------|                                                                           |
 *  | FRAME_ENTRY 3    |------------------------------------------------------------------------------------------]
 *  |------------------|                                                                           |              |
 *  | FRAME_ENTRY N    |                                                                           |              |
 *  |------------------|                                                                           |              |
 *  | FRAME_ENTRY 1024 |                                                                           |              |
 *   ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾                                                                            |              |
 *                                                                                                 |              |
 *  FRAME_ENTRY_2                                                                                  |              |
 *   - Has 32 bits or 4 byte of size                                                               |              |
 *   - Is located at PAGE_TBL_ENTRY_1                                                              |              |
 *   - Starts in RAM at (0x101000 + 0x4 = 0x101004)                                                |              |
 *  __________________________________________________________________________________             |              |
 * | 31 ------------------ 12 | 11 - 9 | 8 | 7  |  6  | 5 |  4  |  3  |  2  |  1  | 0 |------------]              |
 * | Page-Table Base Address  | Availa | G | PS | AVL | A | PCD | PWT | U/S | R/W | P |                           |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾                            |
 *                                                                                                                |
 *  FRAME_ENTRY_3                                                                                                 |
 *   - Has 32 bits or 4 byte of size                                                                              |
 *   - Is located at PAGE_TBL_ENTRY_2                                                                             |
 *   - Starts in RAM at (0x102000 + 0x4 + 0x4 = 0x102008)                                                         |
 *  __________________________________________________________________________________                            |
 * | 31 ------------------ 12 | 11 - 9 | 8 | 7  |  6  | 5 |  4  |  3  |  2  |  1  | 0 |---------------------------]
 * | Page-Table Base Address  | Availa | G | PS | AVL | A | PCD | PWT | U/S | R/W | P |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 * 
 *   ADDITIONAL_INFORMATION:
 *     - The entire structure of the paging config entries composed by PAGE_DIRECTORY with 1024 PAGE_ENTRIES each PAGE_ENTRY has 1024 PAGE_FRAMES
 *     - The total size of the paging mapping configuration structures are:
 *        - PAGE_FRAME: 4 Bytes
 *        - PAGE_TABLE: 4 Kb = 1024 * 4 Bytes
 *        - PAGE_DIR  : 4 Mb = 1024 * 4 Kb
 * 
 *   PAGE_DIRECTORY:
 *     - Is the top level page structure;
 *     - Holds an array of 1024 Page Table entries;
 *     - Those entries can be located in any part of the RAM memory;
 *     - Each entry holds the physical address of a page frame or in a mutilevel paging the physical PAGE_TABLE address Right shifted by 12 bits;
 *     - Each process has it's own page directory;
 * 
 *   PAGE_TABLE:
 *     - Is the second level page structure;
 *     - Holds an array of 1024 Page Frame entries;
 *     - Those entries can be located in any part of the RAM memory;
 *     - Each entry holds the start physical address of the PAGE_FRAME;
 * 
 *   PAGE_FRAME:
 *     - Is the last level page structure;
 *     - Holds the physical memory offset that is the start of the 4kb PAGE_FRAME in RAM memory;
 * 
 * REGISTERS:
 *    cr3 - Points to the current page in memory                 ______________________________
 *          - The virtual address is a 32 bits splitted between | 31---22 | 21------12 | 11--0 |
 *                                                              |PAGE_DIR | PAGE_TABLE | OFFSET|
 *                                                               ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *            PAGE_DIR (Has 1024 pageTableNr)   - Bits: 31-22 = 10 bits = 1024 addresses
 *            PAGE_TABLE (Has 1024 pageNr)      - Bits: 21-12 = 10 bits = 1024 addresses
 *            OFFSET     (Has 4096 frame size)  - Bits: 0-11  = 12 bits = 4096 addresses
 * 
 *    cr0:
 *      - Bit 0 (PE):
 *          - Protected mode enabled
 *          - If 1, system is in protected mode, else system is in real mode
 * 
 *      - Bit 1 (MP):
 *          - Monitor co-processor
 *          - Controls interaction of WAIT/FWAIT instructions with TS flag in CR0
 * 
 *      - Bit 2 (EM):
 *          - Emulation
 *          - If set, no x87 floating-point unit present, if clear, x87 FPU present
 * 
 *      - Bit 3 (TS):
 *          - Task switched
 *          - Allows saving x87 task context upon a task switch only after x87 instruction used
 * 
 *      - Bit 4 (ET):
 *          - Extension type
 *          - On the 386, it allowed to specify whether the external math coprocessor was an 80287 or 80387
 * 
 *      - Bit 5 (NE):
 *          - Numeric error
 *          - Enable internal x87 floating point error reporting when set, else enables PC style x87 error detection
 * 
 *      - Bit 16 (WP):
 *          - Write protect
 *          - When set, the CPU can't write to read-only pages when privilege level is 0
 * 
 *      - Bit 18 (AM):
 *          - Alignment mask
 *          - Alignment check enabled if AM set, AC flag (in EFLAGS register) set, and privilege level is 3
 * 
 *      - Bit 29 (NW):
 *          - Not-write through
 *          - Globally enables/disable write-through caching
 * 
 *      - Bit 30 (CD):
 *          - Cache disable
 *          - Globally enables/disable the memory cache
 * 
 *      - Bit 31 (PG):
 *          - Paging
 *          - If 1, enable paging and use the CR3 register, else disable paging.
 *    
 * 
 * VIRTUAL_ADDRESS:
 *   - The virtual address is calculated based on entry index of the PAGE_DIRECTORY, PAGE_TABLE AND PAGE_FRAME offsets;
 *   
 * 
 * PAGE_DIRECTORY:
 *    - An array of 32-bit page-directory entries (PDEs) contained in a 4-KByte page. 
 *    - Up to 1024 page-directory entries can be held in a page directory.
 * 
 *  __________________________________________________________________________________
 * | 31 ------------------ 12 | 11 - 9 | 8 | 7  |  6  | 5 |  4  |  3  |  2  |  1  | 0 |
 * | Page-Table Base Address  | Availa | G | PS | AVL | A | PCD | PWT | U/S | R/W | P |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *  - Page-Table Base Address:
 *      - Physical memory address.
 * 
 *  - Availa - Reserved and available-to-software bits:
 *      - For all IA-32 processors. Bits 9, 10, and 11 are available for use by software. 
 *        (When the present bit is clear, bits 1 through 31 are available to software, see Figure 3-16.)
 *      - points to a page table, bit 6 is reserved and should be set to 0.
 *      - When the PSE and PAE flags in control register CR4 are set, the processor generates a page fault 
 *        if reserved bits are not set to 0.
 *      - For Pentium II and earlier processors. Bit 7 in a page-table entry is reserved and should be set to 0
 *      - For a page-directory entry for a 4-MByte page, bits 12 through 21 are reserved and must be set to 0.
 *      - For Pentium III and later processors. For a page-directory entry for a 4-MByte page, bits 13 through 21 are reserved 
 *        and must be set to 0
 * 
 *  - Global (G) flag, bit 8:
 *      - (Introduced in the Pentium Pro processor) — Indicates a global page when set.
 *      - When a page is marked global and the page global enable (PGE) flag in register CR4 is set, 
 *        the page-table or page-directory entry for the page is not invalidated in the TLB when register CR3 is 
 *        loaded or a task switch occurs.
 *      - Only software can set or clear this flag.
 *      - For page-directory entries that point to page tables, this flag is ignored and the global 
 *        characteristics of a page are set in the page-table entries.
 *      - See Section 3.12, “Translation Lookaside Buffers (TLBs)”, for more information about the use of this flag
 * 
 *  - Page attribute table index (PAT) flag, bit 7 in page-table entries for 4-KByte 
 *    pages and bit 12 in page-directory entries for 4-MByte pages:
 *      - Selects PAT entry. For processors that support the page attribute table (PAT).
 *      - this flag is used along with the PCD and PWT flags to select an entry in the PAT, 
 *      which in turn selects the memory type for the page. (see Section 10.12, “Page Attribute Table (PAT)”). 
 *      - For processors that do not support the PAT, this bit is reserved and should be set to 0
 * 
 *  - Page size (PS) flag, bit 7 page-directory entries for 4-KByte pages:
 *      - Determines the page size. When this flag is clear, the page size is 4 KBytes 
 *        and the page-directory entry points to a page table.
 *      - When the flag is set, the page size is 4 MBytes for normal 32-bit addressing 
 *        (and 2 MBytes if extended physical addressing is enabled) and the page-directory entry points to a page.
 *      - If the page-directory entry points to a page table, all the pages associated with that page table will be 
 *        4-KByte pages.
 * 
 *  - Dirty (D) flag, bit 6:
 *      - Indicates whether a page has been written to when set.
 *      -  (This flag is not used in page-directory entries that point to page tables.)
 *      - Memory management software typically clears this flag when a page is initially loaded into physical memory.
 *      - The processor then sets this flag the first time a page is accessed for a write operation.
 *      - This flag is “sticky,” meaning that once set, the processor does not implicitly clear it.  
 *      - Only software can clear this flag.
 *      - The dirty and accessed flags are provided for use by memory management software to manage the transfer of pages 
 *        and page tables into and out of physical memory.
 *      - A dirty bit is a bit in memory switched on when an update is made to a page by computer hardware.
 *        When the dirty bit is switched on, the page is modified and can be replaced in memory. 
 *        If it is off, no replacement is necessary since no updates have been made.
 *      - When a block of memory is to be replaced, its corresponding dirty bit is checked to see if the block needs to be written back to secondary memory 
 *        before being replaced or if it can simply be removed. Dirty bits are used by the CPU cache and in the page replacement algorithms of an operating system.
 * 
 *  - Accessed (A) flag, bit 5:
 *      - Indicates whether a page or page table has been accessed (read from or written to) when set
 *      - Memory management software typically clears this flag when a page or page table is initially loaded into 
 *        physical memory. The processor then sets this flag the first time a page or page table is accessed. 
 * 
 *  - Page-level cache disable (PCD) flag, bit 4:
 *      - Controls the caching of individual pages or page tables.
 *      - When the PCD flag is set, caching of the associated page or page table is prevented.
 *      - When the flag is clear, the page or page table can be cached.
 *      - This flag permits caching to be disabled for pages that contain memory-mapped I/O ports or that do not 
 *        provide a performance benefit when cached. 
 *      - The processor ignores this flag (assumes it is set) if the CD (cache disable) flag in CR0 is set.
 * 
 *  - Page-level write-through (PWT) flag, bit 3:
 *      - Controls the write-through or write-back caching policy of individual pages or page tables.
 *      - When the PWT flag is set, write-through caching is enabled for the associated page or page table.
 *      - When the flag is clear, write-back caching is enabled for the associated page or page table.
 *      - The processor ignores this flag if the CD (cache disable) flag in CR0 is set.
 * 
 *  - User/supervisor (U/S) flag, bit 2:
 *      - Specifies the user-supervisor privileges for a page or group of pages 
 *        (in the case of a page-directory entry that points to a page table).
 *      - When this flag is clear, the page is assigned the supervisor privilege level.
 *      - When the flag is set, the page is assigned the user privilege level.
 *      - This flag interacts with the R/W flag and the WP flag in register CR0.
 * 
 *  - Read/write (R/W) flag, bit 1:
 *      - Specifies the read-write privileges for a page or group of pages 
 *        (in the case of a page-directory entry that points to a page table).
 *      - When this flag is clear, the page is read only; 
 *      - When the flag is set, the page can be read and written into.
 *      - This flag interacts with the U/S flag and the WP flag in register CR0.
 * 
 *  - Present (P) flag, bit 0:
 *      - Indicates whether the page or page table being pointed to by the entry is currently loaded in physical memory.
 *      - When set to 1 the page is in physical memory and address translation is carried out.
 *      - When the flag is clear, the page is not in memory and, if the processor attempts to access the page, it generates a 
 *        page-fault exception (#PF)
 *      - The processor does not set or clear this flag; it is up to the operating system or executive to maintain the state of the flag.
 * 
 * - QEMU_DEBUGG:
 *   - Compat monitor:
 *     - info tlb
 *         Show virtual to physical memory mappings
 *     - info mem
 *         Show the active virtual memory mappings
 * 
 *  - SCROLL_UP:
 *     CTRL + PGUP
 */


typedef struct {
    unsigned int present        : 1;
    unsigned int rw             : 1;    // set - r/w, unset - read-only
    unsigned int userMode       : 1;    // set - user mode, unset - kernel mode
    unsigned int reserved1      : 2;
    unsigned int accessed       : 1;
    unsigned int dirty          : 1;    // Set if the page has been written to (dirty)
    unsigned int reserved2      : 2;
    unsigned int unused         : 3;
    unsigned int frameAddress   : 20;   // physical frame address
} __attribute__((packed)) PageTableEntry;

typedef struct
{
    PageTableEntry entry[1024];
} PageTable;

typedef struct
{
    PageTableEntry entry[1024];
} PageDirectory;



namespace paging {
    /**
     * @brief Setup the paging environment, page directories and
     * enable paging in cr0 register
     */
    void install();

    /**
     * @brief Test if paging is working by throwing a page fault
     * 
     */
    void test();

    /**
     * @brief Returns the frame number of the next free frame
     *
     * @return unsigned int   Next free frame number
     */
    unsigned int frameAlloc();

    /**
     * @brief Set the frame number (frameNr) as free to be used by some other process
     *
     * @param frameNr The frame number being free
     */
    void frameFree(unsigned int frameNr);

    /**
     * @brief Given a frameNr set if the frame is in use or free
     *
     * @param frameNr   The number of the frame being modified
     * @param usage     1=in_use, 0=free
     */
    void frameSetUsage(unsigned int frameNr, int usage);

    /**
     * @brief Retrieve an address in RAM memory for given frameNr
     *        Since each frame has 4096 bytes it is used to generate address multiples of 4096 bytes
     *
     * @param frameNr           Frame number to calculate offset
     * @return unsigned int     Frame address
     */
    unsigned int frameAddress(unsigned int frameNr);

    /**
     * @brief Retrieve a PageTableEntry address from a PageDirectory PageTable entry number
     * 
     * @param pageDir           The directory to get the PageTableEntry address
     * @param pageTableNr       The PageTableEntry index inside PageDirectory
     * @return unsigned int     The address of the PageTableEntry
     */
    unsigned int frameAddress(PageDirectory* pageDir, unsigned int pageTableNr);

    /**
     * @brief Retrieve a PageTableEntry address from a PageDirectory PageTable entry number
     * 
     * @param pageDir           The directory to get the PageTableEntry address
     * @param pageTableNr       The PageTableEntry index inside PageDirectory
     * @param pageNr            The PageTableEntry index inside PageTable wich represents the frame
     * @param virtualAddress    The VirtualAddress offset to get specific physical address or use 0 instead
     * @return unsigned int     The address of the PageTableEntry
     */
    unsigned int framePhysicalAddress(PageDirectory* pageDir, unsigned int pageTableNr, unsigned int pageNr, unsigned int virtualAddress);

    /**
     * @brief Retrieve a frame number given a frameAddress in RAM memory
     *
     * @param frameAddress      Frame address in memory to calculate frame number
     * @return unsigned int     Frame number
     */
    unsigned int frameNumber(unsigned int frameAddress);

    /**
     * @brief Retrieve frame amount for a given size of memory
     *
     * @param size              Requested size
     * @return unsigned int     Frame count for given size
     */
    unsigned int sizeInFrames(unsigned int size);

    /**
     * @brief Set the fields of the given page table entry
     *
     * @param tableEntry    The page table entry being configured
     * @param frameAddress  The physical page address
     * @param present       0=Page isn't in RAM,    1=Page is in RAM;
     * @param rw            0=Read Only,            1=Read/Write;
     * @param userMode      0=Supervisor privilege, 1=User privilege
     * 
     */
    void setPageTableEntry(PageTableEntry* tableEntry, unsigned int frameAddress, unsigned int present, unsigned int rw, unsigned int userMode);


    /**
     * @brief Enable the paging by setting bit 31 of cr0 register to 1
     *
     */
    void pagingEnable();

    /**
     * @brief Disable the paging by setting bit 31 of cr0 register to 0
     *
     */
    void pagingDisable();

    /**
     * @brief Set the pageDirectory reference in cr3 register
     * The cr3 register holds the entry of the page that in this case is our PageDirectory ptr
     *
     * @param pageDirectory The start address of page tables entries
     */
    void setPageDirectory(PageDirectory* pageDirectory);

    /**
     * @brief Maps virtual address to physical address
     *        virtualAddr should be 0x1000 aligned
     *
     * @param pageDir       The root structure that holds all PageTables and All Frames
     * @param virtualAddr   The virtual address that will be assigned a physical address
     * @param physicalAddr  The physical address that will be assigned to a virtual address
     */
    void mapPage(PageDirectory* pageDir, unsigned int virtualAddr, unsigned int physicalAddr);


    /**
     * @brief Reset the page to unused, usually when the page is not in memory anymore,
     * release it to be allocated by another process.
     * 
     * @param virtualAddr   Virtual address where the page is located
     */
    void unmapPage(unsigned int virtualAddr);

    /**
     * @brief The same as the mapPage function but without the need of provide the PageDirectory ptr
     * 
     * @param virtualAddr   Virtual address offset of the frame
     * @param physicalAddr  Physical address offset of the frame
     */
    void remoteMapPage(unsigned int virtualAddr, unsigned int physicalAddr);

    /**
     * @brief Flush page table cache.
     * Call the setPageDirectory to set the PageDirectory* in cr3 register 
     * that points to current PageDirectory
     * 
     */
    void pagesRefresh();
}

#endif