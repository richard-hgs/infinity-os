; gdt_flush - load new GDT and reset segments.
; ============================================

[section .text]

[global gdt_flush]

; Set a new gdt specification in intel CPU System
gdt_flush:
	mov eax, [esp+4]	; Receive the first function parameter from stack and save in general purpose register
	lgdt [eax]			; Then load this new parameter as a gdtr entry in CPU System

	mov ax, 0x10		; Reset all registers
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	jmp 0x08:.flush		; Perform a far jump to clear the garbage generated in between registers.
.flush:
	ret					; All done. Transfer control to the stack address placed by a call function. Return to the caller.