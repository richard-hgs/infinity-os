; ==============================================
; gen_interrupt.asm - C-style subroutine
; Receive 1 argument on the stack and use it.
; return nothing.
; Satisfies
;   void gen_interrupt(int);
; ==============================================

section .text
global gen_interrupt

gen_interrupt:
	;preamble
	push ebp
	mov ebp, esp
	
	mov eax, [ebp+8]
	mov [.genint+1], eax
.genint:
	int 0
	hlt
	mov esp, ebp
    pop ebp
	ret