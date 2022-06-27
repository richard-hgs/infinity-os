; ============================================
; Load new GDT and reset segments.
; ============================================

[bits 32]
[section .text]

[global gdt_flush]

; Set a new gdt specification in intel CPU System
gdt_flush:
	mov eax, [esp+4]	; Receive the first function parameter from stack and save in general purpose register
	lgdt [eax]			; Then load this new parameter as a gdtr entry in CPU System

	mov ax, 0x10		; 0x10 is a stand-in for your data segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	jmp 0x08:.flush		; Since we can't use mov instruction to set the cs and ds we use a jmp instruction
						; 0x08 is a stand-in for your code segment located at GDT
						; So here we are Loading 0x08 into cs
.flush:
	ret					; All done. Transfer control to the stack address placed by a call function. Return to the caller.