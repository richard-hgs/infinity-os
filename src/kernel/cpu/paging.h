#pragma once
#ifndef _PAGING_H_
#define _PAGING_H_

#include <stdint.h>

/**
 * @brief MMU - Memory Management Unity - Paging
 *        docs/intel_x86_x64_specification.pdf page 119
 * 
 * Structure that translates linear physical addresses to physical addresses.
 */

// Linker Base Address and Max Address of the kernel
extern unsigned char __BASE_ADDR[];
extern unsigned char __MAX_ADDR[];

/**
 * @brief Page dir structure
 *  ______________________________________________________________________________
 * | 31 ------------------ 12 | 11 - 9 | 8 | 7  |  6  | 5 |  4  |  3  |  2  |  1  |
 * | Page-Table Base Address  | Availa | G | PS | AVL | A | PCD | PWT | U/S | R/W |
 *  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
 *  - Page-Table Base Address:
 *      - The start memory address of the paged memory.
 * 
 *  - Availa - Available:
 *      - Available to software bits 9, 10 and 11.
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
typedef struct pagedir {
    uint8_t access;
    uint8_t base_low;
    uint16_t base_high;
} pagedir_t;

namespace paging {
    void install();
}

#endif