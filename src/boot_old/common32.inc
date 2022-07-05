; Common functions for 32 bit mode.
; by Infinity Technology.
; ============================================

; print function for 32 bit mode.
; ============================================
; ah = text attribute.
; esi = address of data to print.
; ============================================
print32:
	pusha
	mov edx, 0xb8000
.loop:
	mov al, byte [ebx]
	cmp al, 0
	je .done
	mov [edx], ax
	add ebx, 1
	add edx, 2
	jmp short .loop
.done:
	popa
	ret

