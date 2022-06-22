## Infinity OS

- Features:
<pre>
    &#9745; Simple file system to load second stage and kernel during boot process;
    &#9745; Boot process usign two boot stages in asm;
    &#9745; Vga Text mode 80x25;
      &#9745; Print text on screen;
      &#9745; Scroll content up on last line reached and is full filed;
      &#9745; Format strings (kstdlib::kprintf("A String: %s", "My string")):
         &#9745; %s = String data type;
         &#9745; %c = Char data type;
         &#9745; %d = Integer data type;
         &#9745; %x = Hex representation of int data type;
         &#9745; %b = Binary representation of int data type;
         
    &#9744; Process scheduling;
</pre>