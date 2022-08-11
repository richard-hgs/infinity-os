[BITS 16]

ORG 0x7C00

; ==============================================================================================
; FAT32 - Boot Sector definition
;  - BS : Boot Sector
;  - BPB: Bios Parameter Block
                                            ;  _________________________________________________________________________________________________
                                            ; | offset | size | Current Value       |  Description                                              |
jmp START                                   ; |    0   |   3  |                     | Jmp to boot entry point                                   |
BS_OEMName:      db "INFINITY"              ; |    3   |   8  | INFINITY            | Fat OEM signature                                         |
BPB_BytesPerSec: dw 0x200                   ; |   11   |   2  | 512 BYTES           | Bytes per sector                                          |
BPB_SecPerClus:  db 0x1                     ; |   13   |   1  | 1 SECTOR            | Sectors per cluster                                       |
BPB_RsvdSecCnt:  dw 0x1                     ; |   14   |   2  | 1 FAT32 OBLIG       | Reserved sectors                                          |
BPB_NumFATs:     db 0x2                     ; |   16   |   1  | 2 COMPATIBLE        | FAT count                                                 |
BPB_RootEntCnt:  dw 0x0                     ; |   17   |   2  | 0 FAT32 OBLIG       | Amount of dir entries in root dir                         |
BPB_TotSec16:    dw 0x0                     ; |   19   |   2  | 0 FAT32 OBLIG       | 16-bit total count of sectors on the volume               |
BPB_Media:       db 0xf0                    ; |   21   |   1  | f0 REMOVABLE        | Media type                                                |
BPB_FATSz16:     dd 0x0                     ; |   22   |   2  | 0 FAT32 OBLIG       | 16 bit count of sectors used by one fat                   |
BPB_SecPerTrk:   dw 0x12                    ; |   24   |   2  | 18 SECT/TRACK       | Sectors per track                                         |
BPB_NumHeads:    dw 0x1                     ; |   26   |   2  | 1 HEAD              | Read/Write disk heads                                     |
BPB_HiddSec:     dd 0x0                     ; |   28   |   4  | 0 HIDDEN SECT       | Count of hidden sectors                                   |
BPB_TotSec32:    dd 0xB40                   ; |   32   |   4  | 2880 SECTORS        | 32 bit total count of sectors on the volume               |
BPB_FATSz32:     dd 0x1C                    ; |   36   |   4  | 28 SECTORS          | Sectors used by one fat structure                         |
BPB_ExtFlags:    dw 0x80                    ; |   40   |   2  | 0 ACTIVE FAT        | Extended flags. Zero based number of active fat           |
BPB_FSVer:       dw 0x0                     ; |   42   |   2  | 0:0 MAJ/MIN VERSION | File system Major and Minor veresion                      |
BPB_RootClus:    dd 0x2                     ; |   44   |   4  | 2 ROOT FIRST CLUSTE | Root directory first cluster number                       |
BPB_FSInfo:      dw 0x1                     ; |   48   |   2  | 1º SECTOR OF RESERV | Sector number of FSINFO structure in reserved area        |
BPB_BkBootSec:   dw 0x6                     ; |   50   |   2  | 6º SECTOR OF RESERV | Sector number of copy of the boot record in reserved area |
BPB_Reserved:    db 0,0,0,0,0,0,0,0,0,0,0,0 ; |   52   |  12  | RESERVED AREA       | Reserved for future expansion                             |
BS_DrvNum:       db 0x0                     ; |   64   |   1  | DRIVER 0            | Holds the boot drive number                               |
BS_Reserved1:    db 0x0                     ; |   65   |   1  | RESERVED 1          | Reserved, empty                                           |
BS_BootSig:      db 0x29                    ; |   66   |   1  | BOOT SIGNATURE 41   | Extended boot signature                                   |
BS_VolID:        dd 0xface                  ; |   67   |   4  | 0xFACE VOLUME ID    | Disk serial number                                        |
BS_VolLab:       db "Main Volume"           ; |   71   |  11  | Main Volume LABEL   | Volume label max length 11                                |
BS_FilSysType:   db "FAT32   "              ; |   82   |   8  | FAT32 FSystem Type  | File System type                                          |
                                            ;  ‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾‾
; ===============================================================================================

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

    mov ax, 0x10 ; 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov ss, ax
    mov esp, [kernel_stack_pointer]
    jmp 0x8:protected_modeStart

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
; A20 Fast Gate
    pusha
    in al, 0x92
    or al, 2
    out 0x92, al
    popa
    ret
; =====================
    ; pusha
	; mov ax, 0x2400
	; int 0x15
	; popa
	; ret
; =====================
;     pusha
;     cli

;     call a20_waitInput
;     mov al, 0xAD ;disable keyboard
;     out 0x64, al

;     call a20_waitInput
;     mov al, 0xD0 ;copy command byte to 0x60 port
;     out 0x64, al

;     ;call a20_waitOutput
;     in al, 0x60
;     push ax ;save command byte

;     call a20_waitInput
;     mov al, 0xD1 ;next written port to 0x60 is written to 0x64
;     out 0x64, al

;     pop ax
;     or al, 2 ;enable a20
;     call a20_waitInput
;     out 0x64, al

;     call a20_waitInput
;     mov al, 0xAE ;enable keyboard
;     out 0x64, al

;     call a20_waitInput

;     sti
;     popa
; ret

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
    hlt
    jmp fatal_error


boot_disk: db 0
welcome_msg db "InfinityOS bootloader", 13, 10, 0
kernel_stack_pointer dd 0x6504FFF


kernel_start_address dd 0x6400000

[BITS 32]
protected_modeStart:
    mov edi, [kernel_start_address] ;kernel memory
    mov esi, 0x7E00   ; kernel source code
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
    dw 0xFFFF ; segment limiter
    dw 0x0000 ; base
    db 0x00 ; base
    db 10011010b
    db 11001111b
    db 0x0 ; base

gdt_data:
    dw 0xFFFF ; segment limiter
    dw 0x0000 ; base
    db 0x00 ; base
    db 10010010b
    db 11001111b
    db 0x0 ;base

gdt_vram:
    dw 0xFFFF ; segment limiter
    dw 0x8000 ; base
    db 0x0B ; base
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
