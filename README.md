## Infinity OS

- Features:
  - ✅ Architecture x86;
  - ✅ Boot process usign a single stage in assembly;
  - ✅ Vga Text mode 80x25;
    - ✅ Print text on screen;
    - ✅ Scroll content up on last line reached and is full filled;
    - ✅ Supported Scape sequeces:
      - ✅ \n = Line break;
      - ⬜ \t = Tabulation;
      - ⬜ \r = Carriage return (move cursor position to start of the line);
      - ⬜ \b = Backspace;
      - ⬜ \f = Form feed (clear screen);
    - ✅ Format strings (stdlib::kprintf("A String: %s", "My string")):
      - ✅ %s = String data type;
      - ✅ %c = Char data type;
      - ✅ %d = Integer data type;
      - ✅ %x = Hex representation of int data type;
      - ✅ %b = Binary representation of int data type;
  - ⬜ PS/2 Keyboard;
      - ⬜ Key code scan read;
  - ✅ PIT - Programmable Interval Timer;
      - ✅ Minimum implementation, not used yet;
  - ✅ GDT - Global Descriptor Table;
  - ✅ IDT - Interrupt Descriptor Table;
  - ✅ ISR - Interrupt Service Routine;
      - ✅ CPU Interruptions (0-31);
      - ✅ PIC Interruptions (32-47);
      - ✅ Kernel Interruptions (48-255) (Kernel Syscalls);
          - ✅ int 0x30 Syscall that handle all SYSFUNCS;
  - ✅ PIC 8259 - Programmable Interrupt Controller;
      - ✅ Remaped the Master and Slave PIC IRQs vectors to offsets (Master = 0x20), (Slave = 0x28);
      - ✅ Maskable IRQs lines. Mask function implemented to disable/enable IRQs lines from being triggered by hardware and notified to the CPU.
      - ✅ EOI - End Of Interruption. Implemented to clear the In Service Register (ISR).
      - ✅ Possibility to disable the PIC to use in it's place the APIC;
  - ⬜ APIC - Advanced Programmable Interrupt Controller;
  - ✅ CPUID - Central Processing Unit Identification;
     - ✅ Vendor id implemented to get the CPU vendor, like AMD, INTEL, ARM, etc;
     - ✅ Func EAX=1 Fully implemented to get the CPU capabilities;
     - ⬜ Not fully implemented yet;
  - ✅ MMU - Memory Management Unity or Paging;
      - ✅ PageDirs and PageTables configured;
      - ✅ Kernel mapped successfully;
      - ✅ VGA memory mapped successfully;
      - ⬜ I/O ports mapped successfully;
  - ✅ HEAP - Heap system management mechanism;
      - ✅ SHARED - For test purpose the user process heaps are allocated inside kernel heap;
      - ✅ KERNEL - Functions that handle kernel heap. kmalloc and kfree;
      - ✅ PROCESS - Functions that handle user process heap. malloc and free;
  - ✅ VFS - Virtual file system. Since we actually don't have a file system;
      - ✅ findFile - Function to search a file by it's name in virtual file system list;
  - ✅ SYSCALLS - System calls that is executed when a SYSFUNCS is called;
      - ✅ INT 0x30(48) - General Syscall that handle all SYSFUNCS;
          - ✅ EAX 0x01(1) - VGA - printStr
  - ⬜ Programs/Processes/Libs;
      - ✅ SYSFUNCS - System Functions that runs in user mode and perform SYSCALLS;
          - ✅ VGA - Video Graphics Array;
              - ✅ TEXT - printStr - Prints raw text with escape sequences only in the screen;
      - ⬜ Idle process creation;
      - ✅ SHELL process creation;
          - ✅ Communicate with kernel using IDT Interruptions to access VGA memory;
          - ✅ Hello World Print;

      
