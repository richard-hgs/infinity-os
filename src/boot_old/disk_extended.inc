; simple BIOS disk services

; ======================================
; reset disk drive
; ======================================
reset_disk:
	push ax
	xor ax, ax
	mov dl, [iBootDrive]
	int 0x13
	pop ax
	ret

; ==================================================
; Description: Load a file from my file table.
; No parameters.
; ax -> Holds the file_sector_start
; cx -> Holds the file_sector_count
; ==================================================
load_file:
	mov bx, root_segment		; The bottom of the file descriptor table
	mov es, bx					; Save the root_segment address into es data segment register as a pointer
	xor bx, bx					; Clear bx register to 0
.loop:
	mov al, byte [bx]
	or al, al
	jz .error
	mov cx, 11
	mov di, bx
	mov si, op_filename
	repe cmpsb
	jz .found
	add bx, 16
	jmp short .loop
.found:
	mov ax, word [es:bx+0x0c]	; file_name_start + 0x0c = file_sector_start
	mov cx, word [es:bx+0x0e]	; file_name_start + 0x0e = file_sector_count
	ret
.error:
	mov si, op_ferror
	call print
	ret

; read_sectors_16
;
; Reads sectors from disk into memory using BIOS services
;
; input:    dl      = drive
;           ch      = cylinder[7:0]
;           cl[7:6] = cylinder[9:8]
;           dh      = head
;           cl[5:0] = sector (1-63)
;           es:bx  -> destination
;           al      = number of sectors
;
; output:   cf (0 = success, 1 = failure)
read_disk:
    pusha
    mov si, 0x02    ; maximum attempts - 1
.top:
    mov ah, 0x02    ; read sectors into memory (int 0x13, ah = 0x02)
    int 0x13
    jnc .end        ; exit if read succeeded
    dec si          ; decrement remaining attempts
    jc  .end        ; exit if maximum attempts exceeded
    xor ah, ah      ; reset disk system (int 0x13, ah = 0x00)
    int 0x13
    jnc .top        ; retry if reset succeeded, otherwise exit
.end:
    popa
    retn

abs_sector db 0
abs_track db 0
abs_head db 0

