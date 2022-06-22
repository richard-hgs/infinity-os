; all common functions for 16 bit and 32 bit
; by Infinity Technology
; ==========================================

; [ds:si] = message to print.
; ================================
print:
	pusha
.loop:
	lodsb		; load a byte into al
	or al, al ; check al if 0
	jz .done ; jump to done
	mov ah, 0x0e
	mov bh, 0x00
	int 0x10
	jmp short .loop
.done:
	popa
	ret

; ax = hex value to print.
; ================================
print_hex:
	pusha
	mov di, hex16
	mov si, hexstr
	mov cx, 4
.loop:
	rol ax, 4
	mov bx, ax
	and bx, 0x0f
	mov bl, [si+bx]
	mov [di], bl
	inc di
	dec cx
	jnz .loop
	mov si, hex16
	call print
	popa
	ret

; ================================
; Data section for common include.
; ================================

hex16 db "0000",13,10,0
hexstr db "0123456789ABCDEF",0
