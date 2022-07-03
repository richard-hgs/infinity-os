#pragma once
#ifndef _PAGING_H_
#define _PAGING_H_

#include <stdint.h>

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIRECTORY 1024
#define PAGE_SIZE 4096

#define PAGE_MEM_ADDRESS_END 0x1000000 // The size of physical memory. For the moment wE assume it is 16MB big.


/**
 * @brief MMU - Memory Management Unity - Paging
 *        docs/intel_x86_x64_specification.pdf page 119
 * 
 * Structure that translates linear physical addresses to physical addresses.
 * 
 * REGISTERS:
 *    cr3 - Points to the current page directory
 *    cr0 - Bit 31 Set to 1=Enable paging mechanism, 0=Disable paging
 *    
 * 
 * PAGE DIRECTORY:
 *    An array of 32-bit page-directory entries (PDEs) contained in a 4-KByte page. 
 *    Up to 1024 page-directory entries can be held in a page directory.
 * 
 *  __________________________________________________________________________________
 * | 31 ------------------ 12 | 11 - 9 | 8 | 7  |  6  | 5 |  4  |  3  |  2  |  1  | 0 |
 * | Page-Table Base Address  | Availa | G | PS | AVL | A | PCD | PWT | U/S | R/W | P |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *  - Page-Table Base Address:
 *      - The start memory address of the paged memory.
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
 */


typedef struct PageTableEntry {
    uint32_t present         : 1; // Page present in memory
    uint32_t readWrite       : 1; // Read-only if clear, readwrite if set
    uint32_t supervisor      : 1; // Supervisor level only if clear
    // uint32_t writeTrough     : 1; // Has the page been accessed since last refresh?
    // uint32_t cacheDisabled   : 1; // Has the page been written to since last refresh?
    uint32_t accessed        : 1; 
    uint32_t dirty           : 1;
    // bool size            : 1;
    // bool attribute       : 1;
    // bool global          : 1;
    // uint8_t available    : 3;
    uint32_t unused      : 7;
    uint32_t baseAddress : 20;
} __attribute__((packed)) PageTableEntry_t;         // Size 4 bytes (32 bits): Each entry manages 4kb or 4096 bytes of ram wich gives (4gb of ram = 1024 * 1024 * 4096)

typedef struct PageTable {
    // Each page table points to 1024 frames and each frame address 4kb of memory
    PageTableEntry_t pages[PAGES_PER_TABLE];      // Size 4mb
} PageTable_t;

typedef struct PageDirectory {
    // The page directory points to 1024 PageTable_t entries
    PageTable_t* tables[PAGES_PER_DIRECTORY];  // Size 4gb
    /**
     * The physical address of tablesPhysical. This comes into play
     * when we get our kernel heap allocated and the directory
     * may be in a different location in virtual memory.
     */
    uint32_t tablesPhysical[1024];

    /**
     * The physical address of tablesPhysical. This comes into play
     * when we get our kernel heap allocated and the directory
     * may be in a different location in virtual memory.
     */
    uint32_t physicalAddr;
} PageDirectory_t;

namespace paging {
    /**
     * @brief Setup the paging environment, page directories and
     * enable paging in cr0 register
     */
    void install();
}

#endif