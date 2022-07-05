[BITS 16]

ORG 0x7C00

jmp 0x0000:START

; op_a20yes db "A20 is enabled.",10,13,0
; op_a20no db "A20 is disabled.",10,13,0

; ; check a20 line (enabled/disabled)
; ; ========================================
; check_a20:
; 	pusha
; 	mov ax, [0x7dfe]
; 	push bx
; 	mov bx, 0xffff
; 	mov es, bx
; 	pop bx
; 	mov bx, 0x7e0e
; 	cmp ax, [es:bx]
; 	jne .fail
; 	mov ax, [0x7dff]
; 	push bx
; 	mov bx, 0xffff
; 	mov es, bx
; 	pop bx
; 	mov bx, 0x7e0f
; 	cmp ax, [es:bx]
; 	jne .fail
; .okay:
; 	mov si, op_a20yes
; 	call print_bios
; 	jmp short .done
; .fail:
; 	mov si, op_a20no
; 	call print_bios
; .done:
; 	popa
; 	ret

a20_enable:
    pusha
    cli

    call a20_waitInput
    mov al, 0xAD ;disable keyboard
    out 0x64, al

    call a20_waitInput
    mov al, 0xD0 ;copy command byte to 0x60 port
    out 0x64, al

    ;call a20_waitOutput
    in al, 0x60
    push ax ;save command byte

    call a20_waitInput
    mov al, 0xD1 ;next written port to 0x60 is written to 0x64
    out 0x64, al

    pop ax
    or al, 2 ;enable a20
    call a20_waitInput
    out 0x64, al

    call a20_waitInput
    mov al, 0xAE ;enable keyboard
    out 0x64, al

    call a20_waitInput

    sti
    popa
ret

a20_waitOutput: ;wait until keyboard output buffer isn't empty
    pusha
    a20_waitOuputLoop:
    in al, 0x64
    test al, 1
    jnz a20_waitOuputLoop
    popa
ret

a20_waitInput: ;wait until keyboard input buffer isn't empty
    pusha
    a20_waitInputLoop:
    in al, 0x64
    test al, 2
    jnz a20_waitInputLoop
    popa
ret


read_sectors_err_msg: db "ERR: read_sectors", 13, 10, 0
dapack:
        db 0x10
        db 0
.count: dw 0 ; int 13 resets this to # of blocks actually read/written
.buf:   dw 0 ; memory buffer destination address
.seg:   dw 0 ; in memory page zero
.addr:  dq 1 ; skip 1st disk sector which is bootloader, which is loaded by BIOS

read_kernel:
    pusha
    mov ax, 127
    mov [dapack.count], ax
    mov ax, 0x7E00
    mov [dapack.buf], ax

    mov dl, [boot_disk]
    mov si, dapack
    mov ah, 0x42
    int 0x13
    jnc read_kernel_end

    mov si, read_sectors_err_msg
    call print_bios
    jmp fatal_error

    read_kernel_end:
        popa
ret

fatal_error:
    cli
    halt
    jmp fatal_error


boot_disk: db 0
welcome_msg db "InfinityOS bootloader", 13, 10, 0
kernel_stack_pointer dd 0x6504FFF

START:
    xor ax, ax ;ax = 0
    mov es, ax
    mov ds, ax
    mov ss, ax
    mov sp, 0x700 ;stack 512 bytes

    mov [boot_disk], dl ; BIOS fills dl with disk number

    mov si, welcome_msg
    call print_bios

    call a20_enable
    ; call check_a20

    call read_kernel

    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or eax, 1
    mov cr0, eax ;switch to protected mode
    ;sti

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov ss, ax
    mov esp, [kernel_stack_pointer]
    jmp 0x8:protected_modeStart


kernel_start_address dd 0x6400000

[BITS 32]
protected_modeStart:
    mov edi, [kernel_start_address] ;kernel memory
    mov esi, 0x7E00   ;kernel source code
    mov ecx, 0xFE00   ; 127 * 512 bytes
    rep movsb

    cli
    jmp [kernel_start_address]

print_bios:
    pusha
    print_loop:
        lodsb
        or al, al  ;set ZF
        jz print_loopEnd
        mov ah, 0x0E
        int 0x10
        jmp print_loop

    print_loopEnd:
    popa
ret

gdt_start:

gdt_null:
    dd 0
    dd 0

gdt_code:
    dw 0xFFFF ;segment limiter
    dw 0x0000 ;base
    db 0x00 ;base
    db 10011010b
    db 11001111b
    db 0x0 ;base

gdt_data:
    dw 0xFFFF ;segment limiter
    dw 0x0000 ;base
    db 0x00 ;base
    db 10010010b
    db 11001111b
    db 0x0 ;base

gdt_vram:
    dw 0xFFFF ;segment limiter
    dw 0x8000 ;base
    db 0x0B ;base
    db 10010010b
    db 11001111b
    db 0x0 ;base

gdt_end:

gdt_desc: ;gdt descriptor
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
db 0x55
db 0xAA
