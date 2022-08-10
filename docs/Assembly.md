## Assembly 32 bit
- HEAP: 
    - Dynamic allocated memory.
    - malloc, calloc, global, static.

- STACK:
    - The stack is a data structure comprised of elements that is added(push) or removed(pop).
    - PUSH: Add an element to the TOP of the stack
    - POP: Removes the TOP element from the stack.
    - Each element of the stack is assigned a stack address.
    - Elements higher on the stack have a lower address.
    - The stack grows towards lower memory addresses
    
                      LOWER     |‾‾‾‾‾‾‾‾‾‾‾|     ADDRS  
                        ^       |‾‾‾‾‾‾‾‾‾‾‾|       .
                        |       |‾‾‾‾‾‾‾‾‾‾‾|       .
                        |       |‾‾‾‾‾‾‾‾‾‾‾| <--- ADDR 2
                        |       |‾‾‾‾‾‾‾‾‾‾‾| <--- ADDR 1
                      HIGHER     ‾‾‾‾‾‾‾‾‾‾‾  

- STACK FRAME:
    - When a function is called it is set up with a STACK FRAME.
    - 
                                                                        STACK
         _________                  
        |   SF    | <--- Func 4                                     |‾‾‾‾‾‾‾‾‾‾‾|     ADDRS  
        |---------|                                                 |‾‾‾‾‾‾‾‾‾‾‾|       .
        |   SF    | <--- Func 3                                     |‾‾‾‾‾‾‾‾‾‾‾|       .
        |---------|                                                 |‾‾‾‾‾‾‾‾‾‾‾| <--- ADDR 2
        |   SF    | <--- Func 2                            EBP ---> |‾‾‾‾‾‾‾‾‾‾‾| <--- ADDR 1
        |---------|                                                  ‾‾‾‾‾‾‾‾‾‾‾  
        |   SF    | <--- Func 1 ---------------------|                     ^
         ‾‾‾‾‾‾‾‾‾                                   |---------------------|

- FUNCTION STACK EXAMPLE:
    - CODE:
        void func(int x) {
            int a = 0;
            int b = x;
        }
        int main() {
            func(10);
        }
    
    - STACK:
        - ESP is decremented to make room for variables.
        - The operation of pushing variables and decrementing ESP to make room for variables is called
          Function Prologue.
        - The prologue is performed whatever a function is called.
        - [EBP-4] is pointing to our first func local variable that is inialized to 0. (int a = 0;)
          Since the integer on a 32 bit system has 4 bytes we decrement EBP by 4 bytes to make room for the integer.
        - The value of x is above the function stack frame, so we need to use general purpose registers to make a copy of the
          value onto the function stack frame. Values on the stack can't be moved directly to another location.
        - So we perform a mov eax, [EBP + 8] then mov [EBP - 8], eax
        - So [EBP+4] is the EIP that will be executed right after our function is finalized
                         _________ 
Esp decrement   ESP --> |         |
ed to make room  ^      |---------|
for variables.   |      |         |
                 |      |---------|
                 |      |    0    | <-- 4 bytes below the base pointer [EBP - 4] is located our first variable (int a = 0;)
  Stack pointer ESP --> |---------| <-- EBP The base pointer is assigned to the stack
                        | retaddr | <-- 4 byte of the function that will be executed as soon the function as gone out of scope.
                        |---------|
                        |   10    | <-- value 10 being passed to func.
                        |---------|

        

- REGISTERS:
    - Small storage areas of the processor.
    - Stores memory addr, values, anything that can be represented by 4 bytes or less.
    - GENERAL PURPOSE REGISTERS: 
        - EAX:
        - EBX:
        - ECX:
        - EDX:
        - ESI:
        - EDI:
    - RESERVED REGISTERS:
        - EBP:
            - Stack Base: Holds the address in the memory of the lower element of the stack.
        - ESP:
            - Stack Pointer: Holds the address in the memory of the top element of the stack.
        - EIP: 
            - Instruction Pointer: Holds the address in the memory of the current instruction being executed.

- INSTRUCTIONS:
    - 

- DEBUG KERNEL INTERRUPTIONS:

- DATA SIZE:
    - db -> Define Byte        : 1 byte
    - dw -> Define Word        : 2 bytes
    - dd -> Define Double Word : 4 bytes