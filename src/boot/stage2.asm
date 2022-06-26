; stage 2 boot loader.
; by Philip Simonson.
; =======================

[org 0x7e00]
[bits 16]

; global kernel_sectors_readed

start:
	mov [iBootDrive], dl 		; Save disk wee boot from	
	call reset_disk				; Reset the disk position

	; set text mode (80x25)
	mov ax, 0x0003
	int 0x10

	; Enable a20 line then check if a20 line is enabled
	call a20_bios
	call check_a20

	; Show loading message
	mov si, op_loading
	call print

	; mov ax, 0x05
	; mov bx, 0x06
	; cmp ax, bx

	; mov cx, 0x07
	; mov dx, 0x08

	; Load the next file
	call load_file

	; Get the size in sectors of the file
	; mov [kernel_sectors_readed], cx

	; load kernel from sectors
	mov bx, load_segment
	mov es, bx
	xor bx, bx

	inc ax
	mov dx, ax

	mov  al, cl             ; load 20 sector
    ; mov  bx, 0x7E00       ; destination (might as well load it right after your bootloader)
    mov cx, 0x000A          ; cylinder 0, sector 10
	mov cl, dl
    mov dl, [iBootDrive]    ; boot drive
    xor dh, dh              ; head 0
	call read_disk

	; switch on protected mode
	cli						; Disable interruptions
	lgdt [gdt.pointer]		; Load GDTR register with start address of Global Descriptor Table
	mov eax, cr0			; Execute a MOV CR0 instruction that sets the PE flag (and optionally the PG flag) in control register CR0
	or al, 1				
	mov cr0, eax			; set PE (Protection Enable) bit in CR0 (Control Register 0)

	; Perform far jump to selector the code segment of the created GDT
	; The code segment is the first segment right after the Null segment. 
	; Multiply by eight and we have our segment identifyer! 
	jmp dword 0x08:INIT_PM

%include "common_extended.inc"
%include "disk_extended.inc"
%include "a20.inc"
%include "gdt.inc"
%include "boot.inc"

[bits 32]
INIT_PM:
	mov ax, 0x10		; Setup all registers with 10h
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	mov ebp, 0x90000
	mov esp, ebp

	; Jump to kernel entry.asm
	; mov ax, [kernel_sectors_readed]
	call run_offset
	jmp $

%include "common32.inc"

; data
op_loading db "Loading kernel, please wait",0
op_done db "done!",10,13,0
op_a20yes db "A20 is enabled.",10,13,0
op_a20no db "A20 is disabled.",10,13,0
op_progress db 0x2e,0
op_ferror db 10,13,"File not found!",10,13,0
op_filename db "kernel  bin",0

; constants
root_segment equ 0x0ee0		; The bottom of the file descriptor table
load_segment equ 0x1000		; The segment to start loading the kernel
run_offset equ 0x00010000	; The offset where the kernel main is located

; kernel_sectors_readed resw 0x0000  ; Kernel sectors readed
