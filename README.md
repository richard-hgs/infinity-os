## Infinity OS

- Features:
  - ✅ Simple file system to load second stage and kernel during boot process;
  - ✅ Boot process usign two boot stages in asm;
  - ✅ Vga Text mode 80x25;
    - ✅ Print text on screen;
    - ✅ Scroll content up on last line reached and is full filed;
    - ✅ Supported Scape sequeces:
      - ✅ \n = Line break;
      - [ ] \t = Tabulation;
      - [ ] \r = Carriage return (move cursor position to start of the line);
      - [ ] \b = Backspace;
      - [ ] \f = Form feed (clear screen);
      - 
    - ✅ Format strings (kstdlib::kprintf("A String: %s", "My string")):
      - ✅ %s = String data type;
      - ✅ %c = Char data type;
      - ✅ %d = Integer data type;
      - ✅ %x = Hex representation of int data type;
      - ✅ %b = Binary representation of int data type;
       
- [ ] Process scheduling;
