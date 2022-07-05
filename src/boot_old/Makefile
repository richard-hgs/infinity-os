# Bootloader makefile by Infinity Technology
BUILD_DIR=../../build/
CURRENT_DIR=$(shell basename $(CURDIR))

AS=nasm
AFLAGS=-f bin

BACKUPS=$(shell find . -iname "*.bak")
INCLUDES=common.inc common32.inc a20.inc disk.inc disk_extended.inc common_extended.inc
BOOT_ASM_SOURCES := $(shell find './' -type f -name '*.asm')

TARGETS=$(BUILD_DIR)$(CURRENT_DIR)/stage1.bin $(BUILD_DIR)$(CURRENT_DIR)/stage2.bin

.PHONY: all test clean distclean
all: $(TARGETS)

$(BUILD_DIR)$(CURRENT_DIR)/%.bin: %.asm $(INCLUDES)
	mkdir -p $(dir $@)
	$(AS) $(AFLAGS) -o $@ $<
	
# objdump -drwC -Mintel $< > $($<.dump)

test:
	$(info $$var is [${INCLUDES}])

clean:
	rm -f $(TARGETS)

distclean:
ifneq ($(BACKUPS),)
	rm -f $(BACKUPS)
endif
