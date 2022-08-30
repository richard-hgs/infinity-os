## CPP LANGUAGE DEFINITIONS
    - Little Endian:
        Least Significant bytes
        Value (0x4F 0x52) will be stored as (0x52 0x4F)
    - Big Endian:
        Most Significant bytes
        Value (0x4F 0x52) will be stored as (0x4F 0x52)

## PERFORMANCE ANALYSIS OF CPP LANGUAGE IMPLEMENTATIONS

### const_cast:
    - On a char* conversion Initializing a variable:
        Read Only:
            mov DWORD PTR [ebp-0xc],0x0    .rodata
        Becomes:
            mov    eax,ds:0x0  DEFAULT_STR
            DWORD PTR [ebp-0xc], eax
    
    - CPU:
       - 50% more instructions to execute
       - Memory usage stays the same

### VSCODE LINKS:
https://clang.llvm.org/docs/ClangFormatStyleOptions.html