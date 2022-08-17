# BUILD THE FILE SYSTEM APP
BUILD_DIR=../../../../build/
CURRENT_DIR=programs/user/$(shell basename $(CURDIR))
TARGET_DIR=$(BUILD_DIR)$(CURRENT_DIR)
APP_NAME=fs

LIBC_SRC_DIR=../../../libs/libc
STDLIBS_SRC_DIR=../../../kernel/stdlibs
LIBSYS_SRC_DIR=../../libs/user
LIBSTATIC_SRC_DIR=../../libs/static
LIBSYSFUNCS_SRC_DIR=../../libs/user

STDLIBS_B_DIR=$(BUILD_DIR)kernel/stdlibs
LIBSYS_B_DIR=$(BUILD_DIR)programs/libs/user
LIBSTATIC_B_DIR=$(BUILD_DIR)programs/libs/static
LIBSYSFUNCS_B_DIR=$(BUILD_DIR)programs/libs/user

LIBSTATIC_I_DIR=$(LIBSTATIC_B_DIR)/include

DEFAULT_LINK=../linkdefault.ld
HEX_VAR_NAME=$(APP_NAME)_bin


# INCLUDE FILES
# INCLUDE_DIRS := -I$(LIBSYS_SRC_DIR) -I$(STDLIBS_SRC_DIR) -I$(LIBC_SRC_DIR)
INCLUDE_DIRS = -I. -I$(LIBSTATIC_I_DIR) -I$(LIBSYSFUNCS_SRC_DIR)

CCX=g++
CXXFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra -g \
	-O2 -ffunction-sections --entry main -Wl,--gc-sections -Wl,-T$(DEFAULT_LINK) -Wl,-Map=$(TARGET_MAP) \
	$(INCLUDE_DIRS) -L$(LIBSTATIC_B_DIR) -lstatic -L$(LIBSYSFUNCS_B_DIR) -lsysfuncs
LDFLAGS = --Ttext 0x0 --oformat elf32-i386 -m elf_i386
LD = ld

TARGET=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).bin
TARGET_ELF=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).elf
TARGET_MAP=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).map
TARGET_DUMP=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).dump
TARGET_RODATA=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).rodata
TARGET_HEX=$(BUILD_DIR)$(CURRENT_DIR)/$(APP_NAME).bin.h

# APP SOURCE FILES AND OBJECTS
C_SOURCES := $(shell find './' -type f -name '*.cpp')
C_OBJECTS := $(patsubst ./%.cpp,$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o, $(C_SOURCES))
LIB_OBJECTS := -Wl,--whole-archive $(LIBSYSFUNCS_B_DIR)/libsysfuncs.a $(LIBSTATIC_B_DIR)/libstatic.a

.PHONY: all test

all: $(C_OBJECTS)

$(C_OBJECTS) : $(C_SOURCES)
	mkdir -p $(dir $@)
	$(CCX) $(CXXFLAGS) -o $@ $< $(LIB_OBJECTS)
	objdump -drwC -Mintel $@ > $@.dump

test:
	$(info $$var is [${C_OBJECTS}])