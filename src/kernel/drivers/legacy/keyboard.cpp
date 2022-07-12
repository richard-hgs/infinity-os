// cpu
#include "pic.h"
#include "isr.h"
#include "keyboard.h"

void keyboard::install() {
    pic::clearMask(IRQ1);
}