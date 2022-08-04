# BUILD THE SHELL APP
BUILD_DIR=../../../../build/
CURRENT_DIR=programs/libs/$(shell basename $(CURDIR))
TARGET_DIR=$(BUILD_DIR)$(CURRENT_DIR)
KERNEL_SRC_DIR=../../../kernel
LIBC_SRC_DIR=../../../libs/libc/
STDLIBS_SRC_DIR=../../../kernel/stdlibs
STDLIBS_B_DIR=$(BUILD_DIR)kernel/stdlibs

KERNEL_C_SOURCES := $(KERNEL_SRC_DIR)/stdlibs/string.cpp \
				   $(KERNEL_SRC_DIR)/stdlibs/stdlib.cpp \
				   $(KERNEL_SRC_DIR)/stdlibs/stdio.cpp \
				   $(KERNEL_SRC_DIR)/drivers/legacy/vga.cpp \
				   $(KERNEL_SRC_DIR)/sys/io.cpp \
				   $(KERNEL_SRC_DIR)/memory/memutils.cpp
KERNEL_C_OBJECTS := $(patsubst $(KERNEL_SRC_DIR)/%, $(BUILD_DIR)kernel/%, $(KERNEL_C_SOURCES))
KERNEL_C_OBJECTS := $(patsubst %.cpp, %.cpp.o, $(KERNEL_C_OBJECTS))
KERNEL_INCLUDE_DIRS := $(dir $(patsubst %,-I%, $(KERNEL_C_SOURCES)))
KERNEL_INCLUDE_DIRS := $(shell echo $(KERNEL_INCLUDE_DIRS) | xargs -n1 | sort -u | xargs)

# INCLUDE FILES
LIB_INCLUDE_DIRS :=-I$(LIBC_SRC_DIR) -I$(STDLIBS_SRC_DIR)
LIB_INCLUDE_OBJS :=

CCX=g++
CXXFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra -g \
	-I. $(LIB_INCLUDE_DIRS)
CXXFLAGS2 = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra -g \
	-I. -I$(LIBC_SRC_DIR) $(KERNEL_INCLUDE_DIRS)
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
OBJECTS=$(LIBS_C_OBJECTS)
# OBJECTS2 HOLDS THE APP SOURCE OBJECTS + LIB_INCLUDE_OBJECTS
OBJECTS2=$(OBJECTS) 

.PHONY: all test

all: $(KERNEL_C_OBJECTS) $(OBJECTS)

$(KERNEL_C_OBJECTS):
	mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS2) -c -o $@ $(patsubst %.cpp.o, %.cpp, $(patsubst $(BUILD_DIR)kernel/%, $(KERNEL_SRC_DIR)/%, $@))
# objdump -drwC -Mintel $(patsubst %.cpp, %.cpp.o, $(patsubst $(KERNEL_SRC_DIR)/%, $(BUILD_DIR)kernel/%, $@)) > $(patsubst %.cpp, %.cpp.o, $(patsubst $(KERNEL_SRC_DIR)/%, $(BUILD_DIR)kernel/%, $@)).dump

$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o : %.cpp
	mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS) -c -o $@ $<
	objdump -drwC -Mintel $@ > $@.dump
	rm -rf $(BUILD_DIR)programs/user/shell/shell.cpp.o

# $(TARGET) : $(TARGET_ELF)
# 	objcopy -O binary $(TARGET_ELF) $@

# $(TARGET_ELF): $(OBJECTS)
# 	$(LD) $(LDFLAGS) -Map=$(TARGET_MAP) -o $@ $(OBJECTS2)
# 	objdump -drwC -Mintel $^ > $(TARGET_DUMP)
# 	objdump -sj.rodata $^ > $(TARGET_RODATA)

test:
	$(info $$var is [${KERNEL_C_SOURCES}])
# ls -l ${BUILD_DIR}
# $(info $$var is [${LIBS_C_OBJECTS}])