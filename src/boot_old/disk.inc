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
; ==================================================
load_file:
	mov bx, root_segment
	mov es, bx
	xor bx, bx
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
	mov ax, word [es:bx+0x0c]
	mov cx, word [es:bx+0x0e]
	ret
.error:
	mov si, op_ferror
	call print
	ret

; ==================================================
; Description: Load file from disk using LBA.
; ax - LBA (Logical Block Address)
; cx - Number of sectors to read.
; ax   -> [bp+8] param1
; cx   -> [bp+6] param2
; bool -> [bp+4] param3 checkMaxSectors 
; [es:bx] - Location to store at (in memory).
; ==================================================
read_disk:
.read_disk_start:
	mov di, 5
.loop:
	push ax
	push cx
	push bx
	; calculate sector
	mov bx, word [iTrackSect]
	xor dx, dx
	div bx
	inc dx
	mov cl, dl
	; calculate track/head
	mov bx, word [iHeadCnt]
	xor dx, dx
	div bx
	mov ch, al
	xchg dl, dh
	; read sector
	mov ax, 0x0201
	mov dl, byte [iBootDrive]
	pop bx
	int 0x13
	jnc .success
	xor ax, ax
	int 0x13
	dec di
	pop cx
	pop ax
	cmp di, 0
	jne .loop
	mov si, op_ferror
	call print
	jmp $
.success:
	; mov si, op_progress
	; call print
	pop cx
	pop ax

	inc ax

	; push ax
	; mov ax, [bp-8]
	; call print_hex
	; pop ax

	; push ax
	; mov ax, [bp-8]
	; inc ax
	; mov [bp-8], ax
	; pop ax

	; Check if should check max sectors
	; push ax
	; mov ax, [bp+4]
	; cmp ax, 1
	; pop ax
	; je .check_max_sector

	; Increment local stack sector count variable
	; push ax
	; mov ax, [bp-4]
	; add ax, 0x01
	; mov word [bp-4], ax
	; pop ax

	; Check if should check max sectors
	; push ax
	; mov ax, [bp-4]
	; cmp ax, [bp+6]
	; pop ax
	; jg .read_finish

	; cmp ax, 0x1F
	; cmp ax, [bp+4]
	; jg .read_finish

	add bx, word [iSectSize]
	loop .read_disk_start
.read_finish:
	mov si, op_done
	call print

	; pop bp
	ret

abs_sector db 0
abs_track db 0
abs_head db 0

