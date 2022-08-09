# BUILD THE FILE SYSTEM APP
BUILD_DIR=../../../../build/
CURRENT_DIR=programs/user/$(shell basename $(CURDIR))
TARGET_DIR=$(BUILD_DIR)$(CURRENT_DIR)
APP_NAME=shell

LIBC_SRC_DIR=../../../libs/libc
STDLIBS_SRC_DIR=../../../kernel/stdlibs
LIBSYS_SRC_DIR=../../libs/user
LIBSTATIC_SRC_DIR=../../libs/static
LIBSYSFUNCS_SRC_DIR=../../libs/user

STDLIBS_B_DIR=$(BUILD_DIR)kernel/stdlibs
LIBSYS_B_DIR=$(BUILD_DIR)programs/libs/user
LIBSTATIC_B_DIR=$(BUILD_DIR)programs/libs/static
LIBSYSFUNCS_B_DIR=$(BUILD_DIR)programs/libs/user

DEFAULT_LINK=../linkdefault.ld
HEX_VAR_NAME=$(APP_NAME)_bin


# INCLUDE FILES
# INCLUDE_DIRS := -I$(LIBSYS_SRC_DIR) -I$(STDLIBS_SRC_DIR) -I$(LIBC_SRC_DIR)
INCLUDE_DIRS = -I. -I$(LIBSTATIC_SRC_DIR) -I$(LIBSYSFUNCS_SRC_DIR) -I$(LIBC_SRC_DIR)
INCLUDE_OBJS := $(LIBSYS_B_DIR)/sysfuncs.cpp.o $(STDLIBS_B_DIR)/stdlib.cpp.o $(STDLIBS_B_DIR)/string.cpp.o
# INCLUDE_OBJS := $(STDLIBS_B_DIR)/string.cpp.o

CCX=g++
CXXFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra -g \
	-O2 -ffunction-sections --entry _start -Wl,--gc-sections -Wl,-T$(DEFAULT_LINK) -Wl,-Map=$(TARGET_MAP) \
	$(INCLUDE_DIRS) -L$(LIBSTATIC_B_DIR) -lstatic -L$(LIBSYSFUNCS_B_DIR) -lsysfuncs
LDFLAGS = --Ttext 0x0 --oformat elf32-i386 -m elf_i386
LD = ld

# Optimized compilation
# g++ -nostdlib -nostdinc -fno-builtin -fno-pic -Wall -fPIE -O2 -ffunction-sections -Wl,--gc-sections -I../../../libs/libc -I../../libs/user -I../../../kernel/stdlibs --entry main -o fs.o fs.cpp ../../../kernel/stdlibs/string.cpp

# g++ -m32 -nostdlib -nostdinc -fno-builtin -Wall -fPIC -O2 -ffunction-sections -Wl,--gc-sections -I../../../libs/libc -I../../libs/user -I../../../kernel/stdlibs -Wl,-O2 -Wl,--oformat=elf32-i386 -Wl,-melf_i386 --entry main -o fs.o ../../../../build/kernel/stdlibs/string.cpp.o fs.cpp

# objdump -drwC -Mintel fs.o > fs.dump

TARGET=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).bin
TARGET_ELF=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).elf
TARGET_MAP=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).map
TARGET_DUMP=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).dump
TARGET_RODATA=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).rodata
TARGET_HEX=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).bin.h

# APP SOURCE FILES AND OBJECTS
C_SOURCES := $(shell find './' -type f -name '*.cpp')
C_OBJECTS := $(patsubst ./%.cpp,$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o, $(C_SOURCES))
LIB_OBJECTS := $(LIBSYSFUNCS_B_DIR)/libsysfuncs.a $(LIBSTATIC_B_DIR)/libstatic.a

# # OBJECTS HOLDS THE APP SOURCE OBJECTS ONLY
# OBJECTS=$(C_OBJECTS)
# # OBJECTS2 HOLDS THE APP SOURCE OBJECTS + LIB_INCLUDE_OBJECTS
# OBJECTS2=$(OBJECTS) $(INCLUDE_OBJS)

.PHONY: all test

all: $(TARGET)


$(TARGET) : $(TARGET_ELF)
	objcopy -O binary $(TARGET_ELF) $@
	xxd -i $(TARGET) | sed -e 's/unsigned char [a-z_]*/unsigned char $(HEX_VAR_NAME)/g' -e 's/unsigned int [a-z_]*/const unsigned int $(HEX_VAR_NAME)_len/g' > $(TARGET_HEX)
	rm -rf $(BUILD_DIR)kernel/sys/fs.cpp.o

$(TARGET_ELF) : $(C_SOURCES)
	mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS) -o $@ $< $(LIB_OBJECTS)
	objdump -drwC -Mintel $@ > $@.dump
	objdump -f $@ > $@.fdump
	objdump -p $@ > $@.pdump

# $(TARGET) : $(TARGET_ELF)
# 	objcopy -O binary $(TARGET_ELF) $@
# 	xxd -i $(TARGET) | sed -e 's/unsigned char [a-z_]*/unsigned char $(HEX_VAR_NAME)/g' -e 's/unsigned int [a-z_]*/const unsigned int $(HEX_VAR_NAME)_len/g' > $(TARGET_HEX)


# $(TARGET_ELF): $(OBJECTS)
# 	$(LD) $(LDFLAGS) -T$(DEFAULT_LINK) -Map=$(TARGET_MAP) -o $@ $(OBJECTS)
# 	objdump -drwC -Mintel $(OBJECTS2) > $(TARGET_DUMP)
# 	objdump -sj.rodata $^ > $(TARGET_RODATA)
# 	rm -rf $(BUILD_DIR)kernel/sys/fs.cpp.o

test:
	$(info $$var is [${C_OBJECTS}])