; Stage 1 boot loader by Philip Simonson.

[org 0x7c00]
[bits 16]

[section .text]

global _start
_start:
	jmp short main
	nop

%include "bs.inc"

main:
	jmp 0:init
	nop

init:
	xor ax, ax
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7c00
	cld

	mov [iBootDrive], dl
	call reset_disk

	; mov ax, [iBootDrive]
	; call print_hex

	; jmp $

	; load file table
	mov si, op_loading1
	call print

	; read entire file table into memory
	mov ax, 16
	mul word [iRootSize]
	xor dx, dx
	div word [iSectSize]
	mov cx, ax
	mov ax, word [iResSect]
	mov bx, root_segment
	mov es, bx
	xor bx, bx
	call read_disk

	; jmp $

	; load second stage
	mov si, op_loading2
	call print
	call load_file
	mov bx, load_segment
	mov es, bx
	xor bx, bx
	call read_disk

	
	mov dl, [iBootDrive]			; Save the boot driver number that we used
	xor ax, ax						; Set ax to zero and reset ds(Data Segment) and es(Data Segment) registers
	mov ds, ax
	mov es, ax
	jmp run_segment:run_offset      ; Jump to stage2 address located in memory and continue execution from there

%include "common.inc"
%include "disk.inc"

; data
op_loading1 db "Loading file table",0
op_loading2 db "Loading stage 2, please wait",0
op_done db "success!",13,10,0
op_ferror db 10,13,"File not found!",13,10,0
op_progress db 0x2e,0
op_filename db "stage2  bin",0

; constants
root_segment equ 0x0ee0
load_segment equ 0x07e0
run_segment equ 0x0000
run_offset equ 0x7e00

; padding and magic number
times 510-($-$$) db 0
dw 0xaa55
