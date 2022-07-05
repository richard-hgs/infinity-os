; this is all the stuff for testing and enabling a20 line

; check a20 line (enabled/disabled)
; ========================================
check_a20:
	pusha
	mov ax, [0x7dfe]
	push bx
	mov bx, 0xffff
	mov es, bx
	pop bx
	mov bx, 0x7e0e
	cmp ax, [es:bx]
	jne .fail
	mov ax, [0x7dff]
	push bx
	mov bx, 0xffff
	mov es, bx
	pop bx
	mov bx, 0x7e0f
	cmp ax, [es:bx]
	jne .fail
.okay:
	mov si, op_a20yes
	call print
	jmp short .done
.fail:
	mov si, op_a20no
	call print
.done:
	popa
	ret

; enable a20 with BIOS interrupt
; ========================================
a20_bios:
	push ax
	mov ax, 0x2400
	int 0x15
	pop ax
	ret
