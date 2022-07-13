; ============================================
; Configure the generic interruption handler
; ============================================

[bits 32]
[section .text]


[extern isr_handler]        ; Reference isr_handler exported function from isr.cpp file
[extern irq_handler]        ; Reference irq_handler exported function from isr.cpp file
[extern isr48_handler]      ; Reference isr48_handler exported function from isr.cpp file
[global isr_stub_table]     ; Export isr_stub_table vector to be used in isr.cpp file

; NASM - Macros
;
; MACRO_1:
; %macro macro_name 1
;   label_gen_%+%1:
;     mov eax, [%1]
; %endmacro
;
; Explanation:
;   - %macro          : Informs the compiler that we are creating a macro.
;   - macro_name      : Is the name of this macro.
;   - 1               : Is the parameter count of this macro.
;   - label_gen_      : Is the label that this macro will create.
;   - label_gen_%+%1  : Indicates that the label name will be concatenated with the first macro parameter
;   - mov eax, n      : Is the operation that this macro will perform.
;   - %1              : Is the first parameter passed to this macro
;
; MACRO_2:
; my_vector:
;   %assign i 0
;   %rep 32
;   inc word [table+2*i]
;   %assign i i+1
;   %endrep
; 
;   - my_vector             : Is the label name of the vector being created by a NASM macro.
;   - %assign i 0           : Creates the varable i if not exists and initialize with value 0.
;   - %rep 32               : Will generate what is between %rep and %endrep 32 times.
;   - inc word [table+2*i]  : Will generate inc word table 32 times.
;   - $assign i i+1         : This will increment the i by 1 on each loop event in compiler time.
;   - %endrep               : Indicates the end of the repetition macro.

; ===================================================================================================
; MACRO - isr_err_stub:
; This macro will create isr_stub_%index%: label with a function that calls isr_handler
; This handler receives an error code as last pushed argument
; ===================================================================================================
%macro isr_return 1
isr_stub_%+%1:
    ; CPU already pushed some registers for us:
    ; err_code, eip, cs, eflags, useresp, ss
    push byte %1            ; int_no    : The interruption index
    jmp isr_dispatcher      ; Jmp to the function that will dispatch the isr to the kernel handler in correct segment descriptor
%endmacro

; ===================================================================================================
; MACRO - isr_noret:
; This macro will create isr_stub_%index%: label with a function that calls isr_handler
; Error code is always 0 because it's not given by the CPU
; ===================================================================================================
%macro isr_noret 1
isr_stub_%+%1:
    ; CPU already pushed some registers for us:
    ; eip, cs, eflags, useresp, ss
    push byte 0             ; err_code  : Error code if applicable
    push byte %1            ; int_no    : The interruption index
    jmp isr_dispatcher      ; Jmp to the function that will dispatch the isr to the kernel handler in correct segment descriptor
                            
%endmacro

; ===================================================================================================
; MACRO - isr_noret:
; This macro will create isr_stub_%index%: label with a function that calls isr_handler
%macro irq_stub 2
isr_stub_%+%1:
    ; CPU already pushed some registers for us:
    ; eip, cs, eflags, useresp, ss
    push byte %2            ; err_code  : In this case is the IRQs index
    push byte %1            ; int_no    : The interruption index
    jmp irq_dispatcher      ; Jmp to the function that will dispatch the irq to the kernel handler in correct segment descriptor
%endmacro

; =============================
; HANDLER DISPATCHER:
; We created the isr_dispatcher to create a function with common code 
; =============================
isr_dispatcher:
    ; CPU already pushed some registers for us:
    ; eip, cs, eflags, useresp, ss

    ; The isr_noret and isr_return push 2 parameters the int_no(The index of this handler function) and 
    ; The error code returned by the handler
    pusha                   ; ds, edi, esi, ebp, esp, ebx, edx, ecx, eax
	
    mov ax, ds              ; lower 16 bits is in the ds register
	push eax                ; Save the segment descriptor onto stack

	mov ax, 0x10            ; Load the kernel segment descriptor
	mov ds, ax              ; Set the segment register's
	mov es, ax              ; Set the segment register's
	mov fs, ax              ; Set the segment register's
	mov gs, ax              ; Set the segment register's

	push esp                ; Push the old stack frame to the new kernel stack frame since we changed to kernel code segment
	cld                     ; Clear the direction flag
    
    call isr_handler        ; C function to handle ISR's
    
    pop eax                 ; Pop eax that was pushed earlier when we push esp
	pop eax                 ; Pop the segment descriptor from the stack
	mov ds, ax              ; Clear segment register's
	mov es, ax              ; Clear segment register's
	mov fs, ax              ; Clear segment register's
	mov gs, ax              ; Clear segment register's
	popa                    ; pop: ds, edi, esi, ebp, esp, ebx, edx, ecx, eax
	add esp, 8              ; Clean up error code and push number
    iret                    ; An ISR has to end with the iret opcode. Use iretq instead if targeting 64-bit. 
                            ; Concurrent NMIs are delivered to the CPU one by one. IRET signals to the NMI circuitry that another NMI can now be delivered.


; =============================
; HANDLER DISPATCHER:
; We created the irq_dispatcher to create a function with common code 
; =============================
irq_dispatcher:
    ; CPU already pushed some registers for us:
    ; eip, cs, eflags, useresp, ss

    ; The irq_stub push 2 parameters the int_no(The index of this handler function) and 
    ; The error_code in this case is irq index
    pusha                   ; ds, edi, esi, ebp, esp, ebx, edx, ecx, eax
	
    mov ax, ds              ; lower 16 bits is in the ds register
	push eax                ; Save the segment descriptor onto stack

	mov ax, 0x10            ; Load the kernel segment descriptor
	mov ds, ax              ; Set the segment register's
	mov es, ax              ; Set the segment register's
	mov fs, ax              ; Set the segment register's
	mov gs, ax              ; Set the segment register's

	push esp                ; Push the old stack frame to the new kernel stack frame since we changed to kernel code segment
	cld                     ; Clear the direction flag
    
    call irq_handler        ; C function to handle ISR's
    
    pop ebx                 ; Pop ebx that was pushed earlier when we push esp
	pop ebx                 ; Pop the segment descriptor from the stack
	mov ds, bx              ; Clear segment register's
	mov es, bx              ; Clear segment register's
	mov fs, bx              ; Clear segment register's
	mov gs, bx              ; Clear segment register's
	popa                    ; pop: ds, edi, esi, ebp, esp, ebx, edx, ecx, eax
	add esp, 8              ; Clean up error code and push number
    iret                    ; An ISR has to end with the iret opcode. Use iretq instead if targeting 64-bit. 
                            ; Concurrent NMIs are delivered to the CPU one by one. IRET signals to the NMI circuitry that another NMI can now be delivered.


; =============================
; LABEL - isr_stub_table
; This label is a vector, created using a NASM macro
; =============================
isr_stub_table:
%assign i 0
%rep    49
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep

; =========================
; ISRs Below
; =========================
; Here we are invoking the macros created above 
; The parameter we are passing is the isr_stub_table vector index
; The macro creates isr_stub_%index%: label with a function that calls isr_handler then return
;
; isr_noret:  Dont return an error code
; isr_return: Returns an error code
isr_noret 0
isr_noret 1
isr_noret 2
isr_noret 3
isr_noret 4
isr_noret 5
isr_noret 6
isr_noret 7
isr_return 8
isr_noret 9
isr_return 10
isr_return 11
isr_return 12
isr_return 13
isr_return 14
isr_noret 15
isr_noret 16
isr_return 17
isr_noret 18
isr_noret 19
isr_noret 20
isr_noret 21
isr_noret 22
isr_noret 23
isr_noret 24
isr_noret 25
isr_noret 26
isr_noret 27
isr_noret 28
isr_noret 29
isr_return 30
isr_noret 31

; =========================
; IRQs Below
; =========================
; Here we are invoking the macros created above
; The first parameter we are passing is the isr_stub_table vector index
; The second parameter we are passing is the irq offset index
irq_stub 32, 0
irq_stub 33, 1
irq_stub 34, 2
irq_stub 35, 3
irq_stub 36, 4
irq_stub 37, 5
irq_stub 38, 6
irq_stub 39, 7
irq_stub 40, 8
irq_stub 41, 9
irq_stub 42, 10
irq_stub 43, 11
irq_stub 44, 12
irq_stub 45, 13
irq_stub 46, 14
irq_stub 47, 15

isr_stub_48:
    cli
    pusha
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov ss, ax
    
    call isr48_handler
    
    mov eax, 0x10
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    sti
    iret 