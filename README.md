## Infinity OS

- Features:
  - :white_check_mark Simple file system to load second stage and kernel during boot process;
  - :white_check_mark Boot process usign two boot stages in asm;
  - :white_check_mark Vga Text mode 80x25;
    - :white_check_mark Print text on screen;
    - :white_check_mark Scroll content up on last line reached and is full filed;
    - :white_check_mark Format strings (stdlib::kprintf("A String: %s", "My string")):
       - :white_check_mark %s = String data type;
       - :white_check_mark %c = Char data type;
       - :white_check_mark %d = Integer data type;
       - :white_check_mark %x = Hex representation of int data type;
       - :white_check_mark %b = Binary representation of int data type;
       
- [ ] Process scheduling;