# Bootloader makefile by Infinity Technology
BUILD_DIR=../../build/
CURRENT_DIR=$(shell basename $(CURDIR))

AS=nasm
AFLAGS=-f bin -g

BACKUPS=$(shell find . -iname "*.bak")
INCLUDES=
BOOT_ASM_SOURCES := $(shell find './' -type f -name '*.asm')

TARGETS=$(BUILD_DIR)$(CURRENT_DIR)/boot.bin

.PHONY: all test clean distclean
all: $(TARGETS)

$(BUILD_DIR)$(CURRENT_DIR)/boot.bin: boot.asm $(INCLUDES)
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) -o $@ $<
	objdump -DrwC -Mintel,i8086 -b binary -m i386 $@ > $@.dump
	
# objdump -drwC -Mintel $< > $($<.dump)

test:
	$(info $$var is [${INCLUDES}])

clean:
	rm -f $(TARGETS)

distclean:
ifneq ($(BACKUPS),)
	rm -f $(BACKUPS)
endif
