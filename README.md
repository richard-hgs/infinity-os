## Infinity OS

- Features:
  - &#9745; Simple file system to load second stage and kernel during boot process;
  - [x] Boot process usign two boot stages in asm;
  - [x] Vga Text mode 80x25;
    - [x] Print text on screen;
    - [x] Scroll content up on last line reached and is full filed;
    - [x] Format strings (kstdlib::kprintf("A String: %s", "My string")):
       - [x] %s = String data type;
       - [x] %c = Char data type;
       - [x] %d = Integer data type;
       - [x] %x = Hex representation of int data type;
       - [x] %b = Binary representation of int data type;
       
- [ ] Process scheduling;