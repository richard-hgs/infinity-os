# BUILD THE SHELL APP
BUILD_DIR=../../../../build/
CURRENT_DIR=programs/$(shell basename $(CURDIR))
TARGET_DIR=$(BUILD_DIR)$(CURRENT_DIR)

# INCLUDE FILES
LIB_INCLUDE_DIRS :=
LIB_INCLUDE_OBJS :=

CCX=g++
CXXFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra \
	-I. $(LIB_INCLUDE_DIRS)
LDFLAGS = --Ttext 0x0 --oformat binary -m elf_i386
LD = ld

# TARGET=$(BUILD_DIR)$(CURRENT_DIR)/libs.bin
# TARGET_ELF=$(BUILD_DIR)$(CURRENT_DIR)/libs.elf
# TARGET_MAP=$(BUILD_DIR)$(CURRENT_DIR)/libs.map
# TARGET_DUMP=$(BUILD_DIR)$(CURRENT_DIR)/libs.dump
# TARGET_RODATA=$(BUILD_DIR)$(CURRENT_DIR)/libs.rodata

# APP SOURCE FILES AND OBJECTS
LIBS_C_SOURCES := $(shell find './' -type f -name '*.cpp')
LIBS_C_OBJECTS := $(patsubst ./%.cpp,$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o, $(LIBS_C_SOURCES))

# OBJECTS HOLDS THE APP SOURCE OBJECTS ONLY
OBJECTS=$(LIBS_C_OBJECTS) $(LIB_INCLUDE_OBJS)
# OBJECTS2 HOLDS THE APP SOURCE OBJECTS + LIB_INCLUDE_OBJECTS
OBJECTS2=$(OBJECTS)

.PHONY: all test

all: $(OBJECTS)

$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o : %.cpp
	mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS) -c -o $@ $<
	objdump -drwC -Mintel $@ > $@.dump

# $(TARGET) : $(TARGET_ELF)
# 	objcopy -O binary $(TARGET_ELF) $@

# $(TARGET_ELF): $(OBJECTS)
# 	$(LD) $(LDFLAGS) -Map=$(TARGET_MAP) -o $@ $(OBJECTS2)
# 	objdump -drwC -Mintel $^ > $(TARGET_DUMP)
# 	objdump -sj.rodata $^ > $(TARGET_RODATA)

test:
 	$(info $$var is [${LIB_INCLUDE_DIRS}])