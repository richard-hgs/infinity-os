## Infinity OS

- Features:
  - ✅ Architecture x86;
  - ✅ Simple file system to load second stage and kernel during boot process;
  - ✅ Boot process usign two stages in assembly;
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

  - ✅ GDT - Global Descriptor Table;
  - ✅ IDT - Interrupt Descriptor Table;
  - ✅ ISR - Interrupt Service Routine;
      - ✅ Handle Interruptions (0-31);
      - ⬜ User Interruptions (syscalls);
  - ⬜ MMU - Paging;
  - ⬜ Process scheduling;
      
