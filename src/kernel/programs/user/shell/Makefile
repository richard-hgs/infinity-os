# BUILD THE SHELL APP
BUILD_DIR=../../../../../build/
CURRENT_DIR=programs/user/$(shell basename $(CURDIR))

CCX=g++
CXXFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra
LDFLAGS = --Ttext 0x0 --oformat binary -m elf_i386
LD = ld

TARGET=$(BUILD_DIR)$(CURRENT_DIR)/shell.bin
TARGET_ELF=$(BUILD_DIR)$(CURRENT_DIR)/shell.elf
TARGET_MAP=$(BUILD_DIR)$(CURRENT_DIR)/shell.map
TARGET_DUMP=$(BUILD_DIR)$(CURRENT_DIR)/shell.dump
TARGET_RODATA=$(BUILD_DIR)$(CURRENT_DIR)/shell.rodata

# APP SOURCE FILES AND OBJECTS
SHELL_C_SOURCES := $(shell find './' -type f -name '*.cpp')
SHELL_C_OBJECTS := $(patsubst ./%.cpp, $(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o, $(SHELL_C_SOURCES))

# OBJECTS HOLDS THE APP SOURCE OBJECTS ONLY
OBJECTS=$(SHELL_C_OBJECTS)
# OBJECTS2 HOLDS THE APP SOURCE OBJECTS + LIB_INCLUDE_OBJECTS
OBJECTS2=$(OBJECTS)

.PHONY: all test

all: $(TARGET)

$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o : %.cpp %.h
	mkdir -p $(dir $@)
	$(CCX) $(CFLAGS) -c -o $@ $< $(CFLAGSPOS)

$(TARGET) : $(TARGET_ELF)
	objcopy -O binary $(TARGET_ELF) $@

$(TARGET_ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -Map=$(TARGET_MAP) -o $@ $(OBJECTS2)
	objdump -drwC -Mintel $^ > $(TARGET_DUMP)
	objdump -sj.rodata $^ > $(TARGET_RODATA)

test:
 	$(info $$var is [${SHELL_C_OBJECTS}])