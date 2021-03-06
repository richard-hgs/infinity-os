# BUILD THE SHELL APP
BUILD_DIR=../../../../build/
CURRENT_DIR=programs/user/$(shell basename $(CURDIR))
TARGET_DIR=$(BUILD_DIR)$(CURRENT_DIR)

LIBC_SRC_DIR=../../../libs/libc/
STDLIBS_SRC_DIR=../../../kernel/stdlibs
LIBSYS_SRC_DIR=../../libs/sys/

STDLIBS_B_DIR=$(BUILD_DIR)kernel/stdlibs
LIBSYS_B_DIR=$(BUILD_DIR)programs/libs/sys

DEFAULT_LINK=../linkdefault.ld
HEX_VAR_NAME=shell_bin


# INCLUDE FILES
SHELL_INCLUDE_DIRS := -I$(LIBSYS_SRC_DIR) -I$(STDLIBS_SRC_DIR)
SHELL_INCLUDE_OBJS := $(LIBSYS_B_DIR)/sysfuncs.cpp.o $(STDLIBS_B_DIR)/stdlib.cpp.o $(STDLIBS_B_DIR)/string.cpp.o

CCX=g++
CXXFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra -g \
	-I. $(SHELL_INCLUDE_DIRS)
LDFLAGS = --Ttext 0x0 --oformat elf32-i386 -m elf_i386
LD = ld

TARGET=$(BUILD_DIR)$(CURRENT_DIR)/shell.bin
TARGET_ELF=$(BUILD_DIR)$(CURRENT_DIR)/shell.elf
TARGET_MAP=$(BUILD_DIR)$(CURRENT_DIR)/shell.map
TARGET_DUMP=$(BUILD_DIR)$(CURRENT_DIR)/shell.dump
TARGET_RODATA=$(BUILD_DIR)$(CURRENT_DIR)/shell.rodata
TARGET_HEX=$(BUILD_DIR)$(CURRENT_DIR)/shell.bin.h

# APP SOURCE FILES AND OBJECTS
SHELL_C_SOURCES := $(shell find './' -type f -name '*.cpp')
SHELL_C_OBJECTS := $(patsubst ./%.cpp,$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o, $(SHELL_C_SOURCES))

# OBJECTS HOLDS THE APP SOURCE OBJECTS ONLY
OBJECTS=$(SHELL_C_OBJECTS)
# OBJECTS2 HOLDS THE APP SOURCE OBJECTS + LIB_INCLUDE_OBJECTS
OBJECTS2=$(OBJECTS) $(SHELL_INCLUDE_OBJS)

.PHONY: all test

all: $(TARGET)

$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o : %.cpp
	mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS) -c -o $@ $<

$(TARGET) : $(TARGET_ELF)
	objcopy -O binary $(TARGET_ELF) $@
	xxd -i $(TARGET) | sed -e 's/unsigned char [a-z_]*/unsigned char $(HEX_VAR_NAME)/g' -e 's/unsigned int [a-z_]*/const unsigned int $(HEX_VAR_NAME)_len/g' > $(TARGET_HEX)


$(TARGET_ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -T$(DEFAULT_LINK) -Map=$(TARGET_MAP) -o $@ $(OBJECTS2)
	objdump -drwC -Mintel $(OBJECTS2) > $(TARGET_DUMP)
	objdump -sj.rodata $^ > $(TARGET_RODATA)
	rm -rf $(BUILD_DIR)kernel/sys/fs.cpp.o

test:
	ls -l $(STDLIBS_SRC_DIR) \
	$(info $$var is [${OBJECTS2}])