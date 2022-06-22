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

println:
	call print
	call printnl
	ret

printnl:
	pusha

	mov ah, 0x0e
	mov al, 0x0a ; newline char
	int 0x10
	mov al, 0x0d ; carriage return
	int 0x10

	popa
	ret

; ; ax = hex value to print.
; ; ================================
; print_hex:
; 	pusha
; 	mov di, hex16
; 	mov si, hexstr
; 	mov cx, 4
; .loop:
; 	rol ax, 4
; 	mov bx, ax
; 	and bx, 0x0f
; 	mov bl, [si+bx]
; 	mov [di], bl
; 	inc di
; 	dec cx
; 	jnz .loop
; 	mov si, hex16
; 	call print
; 	popa
; 	ret

;; ==============================================================================
;; Move the cursor to specified position
;; Args:                     
;;   - Parameter1: 16 bits size, 0x0104 (8 Bits high -> ROW, 8 Bits low -> COL);
;; Return:
;;   - NO_RETURN
;; ==============================================================================
moveCursor:
    push bp
    mov bp, sp

    mov dx, [bp+4]          ; get the argument from the stack. |bp| = 2, |arg| = 2
    mov ah, 0x02            ; set cursor position
    mov bh, 0x00            ; page 0 - doesn't matter, we're not using double-buffering
    int 0x10

    ; mov [rowPos], dh        ; DX First Byte as DH is the ROW -> Save into variable
    ; mov [colPos], dl        ; DX Second Byte as DL is the COL - Save into variable

    pop bp
    ret

;; ==============================================================================
;; Get the cursor position
;; Args:                     
;;   - Parameter1: 16 bits size, 0x0104 (8 Bits high -> ROW, 8 Bits low -> COL);
;; Return:
;;   - NO_RETURN
;; ==============================================================================
getCursorPosition:
	push bp
	mov bp, sp
	push cx

	xor ax, ax
	mov ah, 0x03		; Get cursor position
	mov bh, 0x00		; page 0 - doesn't matter, we're not using double-buffering
	int 0x10

	mov [bp+4], dx		; Line and row positions

	pop cx
	pop bp
	ret

;; =================================================================================
;; Print the current state of the 16bit flags register in binary form in the screen.
;; Args:
;;   - NO_ARGS
;; Return:
;;   - NO_RETURN
;; =================================================================================
printFlags:
    ; push 0x01
    ; push flagsH
    ; call print

    ; push 0x01
    ; push flagsB
    ; call print
	push di
	push si
	push ax
	push bx

	push ax
	mov ax, bx
    
	pushf                       ; Push FLAGS register to the stack

	call printnl

	mov si, flagsAndRegsTitle
	call println

	mov si, flagsH
	call println

	mov si, flagsB
	call println

    pop ax                      ; Pop FLAGS register from stack to AX
    ; Note that MOV AX, EFLAGS doesn't work for some reason

    ; Call the AX register to binary display routine
    call ax2bin
	push ax
	popf
	; pop ax

	; Get cursor position and fix its position because of the ax2bin print
	push dx
	push dx
	call getCursorPosition
	
	push ax
	mov ax, dx

	add ax, 0x0010
	push ax
	call moveCursor
	pop ax
	pop ax

	pop dx
	pop dx

	pop bx
	pop bx

	; Add left space from the start position of the
	mov si, flagsRegsSpace
	call print

	; Print ax register contents
	; push ax
	call print_hex_word
	pop ax
	
	mov si, regsSpace
	call print

	push bx
	call print_hex_word
	pop bx

	mov si, regsSpace
	call print

	push cx
	call print_hex_word
	pop cx

	mov si, regsSpace
	call print

	push dx
	call print_hex_word
	pop dx

	mov si, regsSpace
	call print

	pop si
	push si
	call print_hex_word
	pop si

	mov si, regsSpace
	call print

	pop di
	push di
	call print_hex_word
	pop di

	mov si, regsSpace
	call print

	; pop di
	push es
	call print_hex_word
	pop es

	; pop ax
	
	call printnl

	mov si, flags0
	call println
	
	mov si, flags1
	call println

	mov si, flags2
	call println

	mov si, flags3
	call println

	mov si, flags4
	call println

	mov si, flags5
	call println

	mov si, flags6
	call println

	mov si, flags7
	call println

	mov si, flags8
	call println

	mov si, flags9
	call println
	
	mov si, flags10
	call println

	mov si, flags11
	call println
	
	mov si, flags12
	call println
	
	mov si, flags13
	call println
	
	mov si, flags14
	call println

	mov si, flags15
	call println

	; pop si  
	ret
	; call printnl

    ; call moveCursorNextRow
    ; call moveCursorNextRow
    
    ; push 0x01
    ; push flags0
    ; call print

; ====================================================
; Prints the ax register in the screen in binary form
; Args:
;   - AX: The content to be printed
; Return:
;   - NO_RETURN
; ====================================================
ax2bin:
    push cx
    push dx
	mov bx, 0x0
    mov cx, 16
.top:
    rcr ax, 1       ; Rotate and set the carry with what we rotated off
    mov dl, "0"     ; Set DL to "0", the default value
    adc dl, 0       ; If the carry bit was set by the RCR, increase the "0" byte by one to "1"

    push ax         ; Save AX
    mov ah, 0x0A    ; 0x0A is the "print byte at cursor" argument to INT 0x10
    mov al, dl      ; Our result is in DL, but INT 0x10 uses AL
    int 0x10        ; Print the byte
    pop ax          ; Restore AX

    ; Save into global variabless
    ; mov [rowPos], dh
    ; mov [colPos], dl

    loop .top

    pop dx
    pop cx
    ret

print_hex_word:
    push bp
    mov bp, sp      ; BP=SP, on 8086 can't use sp in memory operand
    push dx         ; Save all registers we clobber
    push cx
    push bx
    push ax

    mov cx, 0x0404  ; CH = number of nibbles to process = 4 (4*4=16 bits)
                    ; CL = Number of bits to rotate each iteration = 4 (a nibble)
    mov dx, [bp+4]  ; DX = word parameter on stack at [bp+4] to print
    mov bx, [bp+6]  ; BX = page / foreground attr is at [bp+6]

.loop:
    rol dx, cl      ; Roll 4 bits left. Lower nibble is value to print
    mov ax, 0x0e0f  ; AH=0E (BIOS tty print),AL=mask to get lower nibble
    and al, dl      ; AL=copy of lower nibble
    add al, 0x90    ; Work as if we are packed BCD
    daa             ; Decimal adjust after add.
                    ;    If nibble in AL was between 0 and 9, then CF=0 and
                    ;    AL=0x90 to 0x99
                    ;    If nibble in AL was between A and F, then CF=1 and
                    ;    AL=0x00 to 0x05
    adc al, 0x40    ; AL=0xD0 to 0xD9
                    ; or AL=0x41 to 0x46
    daa             ; AL=0x30 to 0x39 (ASCII '0' to '9')
                    ; or AL=0x41 to 0x46 (ASCII 'A' to 'F')
    int 0x10        ; Print ASCII character in AL
    dec ch
    jnz .loop       ; Go back if more nibbles to process

    pop ax          ; Restore registers
    pop bx
    pop cx
    pop dx
    pop bp
    ret

; ================================
; Data section for common include.
; ================================
regsSpace: db "   ", 0
flagsRegsSpace: db "            ", 0
flagsAndRegsTitle: db "      FLAGS                                     REGISTERS", 0
flagsH:     db "FEDCBA9876543210             AX     BX     CX     DX     SI     DI     ES     ", 0
flagsB:     db "||||||||||||||||             ||     ||     ||     ||     ||     ||     ||     ", 0
flags0:   	db "0 ->   CF   - (S) Carry Flag: 0=NC(NoCarry) 1=CY(Carry)", 0
flags1:   	db "1 ->        -     Reserved: always 1 in EFLAGS", 0
flags2:   	db "2 ->   PF   - (S) Parity flag: 0=PO(Parity Odd) 1=PE(Parity Even)", 0
flags3:   	db "3 ->        -     Reserved:", 0
flags4:   	db "4 ->   AF   - (S) Adjust flag: 0=NA(No Auxiliary Carry) 1=AC(Auxiliary Carry)", 0
flags5:   	db "5 ->        -     Reserved:", 0
flags6:   	db "6 ->   ZF   - (S) Zero flag: 0=NZ(Not Zero) 1=ZR(Zero)", 0
flags7:   	db "7 ->   SF   - (S) Sign flag: 0=PL(Positive) 1=NG(Negative)", 0
flags8:   	db "8 ->   TF   - (C) Trap flag: single step", 0
flags9:   	db "9 ->   IF   - (C) Interrupt enable flag: 0=DI(Disable Interrupt) 1=EI(Enable Interrupt)", 0
flags10:   	db "A ->   DF   - (C) Direction flag: 0=UP(Up) 1=DN(Down)", 0
flags11:   	db "B ->   OF   - (S) Overflow flag: 0=NV(Not Overflow) 1=OV(Overflow)", 0
flags12:   	db "C ->  IOPL  -     I/O Privilege lvl: always 1 in 8086 and 186", 0
flags13:   	db "D ->  IOPL  -     I/O Privilege lvl: always 1 in 8086 and 186", 0
flags14:   	db "E ->   NT   -     Nested task flag: always 1 in 8086 and 186", 0
flags15:   	db "F ->   NT   -     Reserved: always 1 in 8086 and 186, always 0 in others models", 0
hex16 		db "0000",0
hexstr 		db "0123456789ABCDEF",0
