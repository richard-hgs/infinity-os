; ============================================
; Configure the generic interruption handler
; ============================================

[bits 32]
[section .text]


[extern isr_handler]        ; Reference isr_handler exported function from isr.cpp file
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
; This macro will create isr_stub_%index%: label with a function that calls isr_handler then return
; ===================================================================================================
%macro isr_err_stub 1
isr_stub_%+%1:
    ; Push the idt index to the isr_handler as a uint8_t parameter
    push %1
    call isr_handler
    iret            ; use iretq instead if targeting 64-bit
%endmacro

; ===================================================================================================
; MACRO - isr_no_err_stub:
; This macro will create isr_stub_%index%: label with a function that calls isr_handler then return
; ===================================================================================================
%macro isr_no_err_stub 1
isr_stub_%+%1:
    ; Push the idt index to the isr_handler as a uint8_t parameter
    push byte %1
    call isr_handler
    iret
%endmacro

; =============================
; LABEL - isr_stub_table
; This label is a vector, created using a NASM macro
; =============================
isr_stub_table:
%assign i 0
%rep    32
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep

; =========================
; ISRs Below
; =========================
; Here we are invoking the macros created above 
; The parameter we are passing is the isr_stub_table vector index
; The macro creates isr_stub_%index%: label with a function that calls isr_handler then return
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31