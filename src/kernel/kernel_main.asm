; ================================================
; entry.asm - Kernel Entrypoint.
; ================================================

[bits 32]

[extern kmain]
[global _start]

_start:
	; Save the kernel sectors readed from ax to use later
	; mov [kernel_sectors_readed], ax

	pusha	;push all registers
	pushf	;push all flags

	; arg1 -> kernel_sectors_readed
	; mov ax, [kernel_sectors_readed]
	; push ax
	; call main kernel function
	call kmain

	popf	; pop all flags
	popa	; pop all registers
	ret		; return to calling function

kernel_sectors_readed resw 0x0000
