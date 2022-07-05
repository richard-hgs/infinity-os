; Global Descriptor Table x86 and x86_64
;  __________________________________________________________________
; |   bits    |    Function   | Description		 					 |
; |  [ 0-15]  |  Limit 0:15   | First 16 bits in the segment limiter |
; |  [16-31]  |  Base 0:15    | First 16 bits in the base address    |
; | -----------------------------------------------------------------|
; |  [32-39]  |  Base 16:23   | Bits 16-23 in the base address       |
; |  [40-47]  |  Access Byte  | Segment type and attributes			 |
; |  [48-51]  |  Limit 16:19  | Bits 16-19 in the segment limiter    |
; |  [52-55]  |  Flags   	  |										 |
; |  [56-63]  |  Base 24:31   |                                      |
;  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
; Functions: 
; - bits 0-15: Segment Limit. Allows 65.536 bytes or 64 KB max length.
; - bits 16-31: Base address of this segment. Allows addresses from 0-65.535 Max ram size of 64kb.
; - bits 32-39: Extends segment address size. Allows addresses from 0-16.777.216 Max ram size of 16384 KB or 16 MB.
; - bits 40-47: Access permissions of the current segment.
; - bits 48-51: Extends segment Limit from 16 bits to 19 bits long. Allows 1.048.576 bytes or 1024 KB or 1 GB max length.
; - bits 52-55: Flags of this segment, defines the specifications of this segment and it's capabilities and state.
; - bits 56-63: Extends segment address size. Allows addresses from 0-4.294.967.295 Max ram size of 4 GB.
;
; Segment Visual Configuration:
;  _______________________________________________________________________________________________________________________________
; | 31 ------------- 24 | 23 |  22 | 21 | 20  | 19 -------------- 16 | 15 | 14 | 13 | 12 | 11 |  10 | 9 | 8 | 7 --------------- 0 |
; | Base Address[31:24] | G  | D/B | L  | AVL | Segment Limit[19:16] | P  |   DPL   | S  |Type| C/E |R/W| A | Base Address[23:16] |  
; |------------------------------------------------------------------|------------------------------------------------------------|
; | Base Address[15:0]                                               | Segment Limit[15:0]										  |
;  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
; Where the fields stands for:
; - Base Address: 
;     - 32 bit starting memory address of the segment.
;
; - Segment Limit:
;     - 20 bit length of the segment. (More specifically, the address of the last accessible data, so the length is one more 
;       that the value stored here.) How exactly this should be interpreted depends on other bits of the segment descriptor.
;
; - G=Granularity:
;     - If clear, the limit is in units of bytes, with a maximum of 2^20 bytes. If set, the limit is in units of 4096-byte pages, for a maximum of 2^32 bytes.
;     - G=0, segments can be 1 byte to 1MB in length.
;     - G=1, segments can be 4KB to 4GB in length.
;
; - D=Default operand size:
;     - If clear, this is a 16-bit code segment; if set, this is a 32-bit segment.
;
; - B=Big: 
;     - If set, the maximum offset size for a data segment is increased to 32-bit 0xffffffff 4gb. Otherwise it's the 
;       16-bit max 0x0000ffff 64kb. Essentially the same meaning as "D".
;
; - L=Long:
;     - If set, this is a 64-bit segment (and D must be zero), and code in this segment uses the 64-bit instruction encoding. 
;       "L" cannot be set at the same time as "D" aka "B".
;
; - AVL=Available:
;     - For software use, not used by hardware.
;     - User (OS) defined bit
;
; - P=Present:
;     - If clear, a "segment not present" exception is generated on any reference to this segment.
;       Descriptor is undefined.
;     - If 1, descriptor contains a valid base and limit.
;
; - DPL=Descriptor privilege level:
;     - Privilege level (ring) required to access this descriptor.
;     - Is the privilege level of a segment. It defines the minimum1 privilege level required to access the segment.
;     - Privilege levels range from 0-3; lower numbers are more privileged.
;
; - S:
;     - 0 = System Descriptor.
;     - 1 = Code, data or stack.
;
; - Type:
;     - If set, this is a code segment descriptor. If clear, this is a data/stack segment descriptor, which has "D" replaced by "B", 
;       "C" replaced by "E" and "R" replaced by "W". This is in fact a special case of the 2-bit type field, where the preceding 
;       bit 12 cleared as "0" refers to more internal system descriptors, for LDT, LSS, and gates.
;
; - C=Conforming: 
;     - Code in this segment may be called from less-privileged levels.
;
; - E=Expand-Down:
;     - If clear, the segment expands from base address up to base+limit. If set, it expands from maximum offset down to limit, 
;       a behavior usually used for stacks.
;
; - R=Readable:
;     - If clear, the segment may be executed but not read from.
;
; - W=Writable:
;     - If clear, the data segment may be read but not written to.
;
; - A=Accessed:
;     - This bit is set to 1 by hardware when the segment is accessed, and cleared by software.
; 
; - BITS(10, 9, 8)(Type, C/E, R/W):
;     - 000 Data read-only
;     - 001 Data read/write
;
;     - 010 Stack read-only
;     - 011 Stack read/write
;
;     - 100 Code, execute-only
;     - 101 Code, execute/read
;
;     - 110 Code, execute-only, conforming
;     - 111 Code, execute/read, conforming
gdt:				; Offset 0x00
.null_descriptor:	; 64 bits
	dq 0			
.code:      		; 
	dw 0xffff		; Segment Limit[0:15] 0xFFFF = 64 KB max length
	dw 0			; Base Address[0:15]  0 = Start of memory
	db 0			; Base Address[16:23] 0 = Start of memory
	db 10011010b 	; [7=(Set to 1 since this is a valid descriptor), [6,5]=DPL(Set to 00 highest privilege level), 4=S(Set to 1 is a code segment descriptor), 3=Type(Set to 1 is a code segment descriptor), 2=C/E(Set to 0 prevent user programs from jumping in this segment), 1=R/W(Set to 1 Read and Write allowed), 0=A(Set to 0 because this bit is set by cpu when segment is accessed)]
	db 11001111b	; [7=G(Set to 1 segments can be 4KB to 4GB in length), 6=D/B(Set to 1 is a 32 bits segment), 5=L Long(Set to 0 since is x32 bit an not x64), 4=AVL(Set to 0 OS don't use this bit), [3,2,1,0]=Segment Limit[16:19](Set to 1111 allow 4gb max segment length)]
	db 0			; Base Address[24:31] 0 = Start of memory
.data:
	dw 0xffff		; Segment Limit[0:15] 0xFFFF = 64 KB max length
	dw 0			; Base Address[0:15]  0 = Start of memory
	db 0			; Base Address[16:23] 0 = Start of memory
	db 10010010b	; [7=(Set to 1 since this is a valid descriptor), [6,5]=DPL(Set to 00 highest privilege level), 4=S(Set to 1 is a data segment descriptor), 3=Type(Set to 0 is a data segment descriptor), 2=C/E(Set to 0 prevent user programs from jumping in this segment), 1=R/W(Set to 1 Read and Write allowed), 0=A(Set to 0 because this bit is set by cpu when segment is accessed)]
	db 11001111b	; [7=G(Set to 1 segments can be 4KB to 4GB in length), 6=D/B(Set to 1 is a 32 bits segment), 5=L Long(Set to 0 since is x32 bit an not x64), 4=AVL(Set to 0 OS don't use this bit), [3,2,1,0]=Segment Limit[16:19](Set to 1111 allow 4gb max segment length)]
	db 0			; Base Address[24:31] 0 = Start of memory
.pointer:
	dw $-gdt-1		; Size of the gdt = $ -> current offset - gdt offset - 1. 
	dd gdt			; Start address of the gdt.
