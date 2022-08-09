BUILD_DIR=../../../../build/
CURRENT_DIR=programs/libs/$(shell basename $(CURDIR))
LIB_NAME=sysfuncs

LIBC_SRC_DIR=../../../libs/libc/
LIBSTATIC_SRC_DIR=../static

INCLUDE_DIRS := -I. -I$(LIBC_SRC_DIR) -I$(LIBSTATIC_SRC_DIR)

C_SOURCES := $(shell find './' -type f -name '*.cpp')
C_OBJECTS := $(patsubst ./%.cpp,$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o, $(C_SOURCES))

COMPILE_OPTIONS=-D__USERMODE
CCX=g++
CCXFLAGS=-static -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra -g \
	-ffunction-sections \
	$(COMPILE_OPTIONS) \
	$(INCLUDE_DIRS)

TARGET=$(BUILD_DIR)$(CURRENT_DIR)/lib$(LIB_NAME).a

.PHONY: all test

all: $(TARGET)

$(TARGET) : $(C_OBJECTS)
	ar --target=elf32-i386 rcs $(TARGET) $(C_OBJECTS)
	objdump -drwC -Mintel $(TARGET) > $(TARGET).dump

# objdump -f $(TARGET) > $(TARGET).elfdump
# objdump -drwC -Mintel $(TARGET) > $(TARGET).dump

$(C_OBJECTS) : $(C_SOURCES)
	mkdir -p $(dir $@)
	$(CCX) $(CCXFLAGS) -c -o $@ $<

test:
 	$(info $$var is [${C_OBJECTS}])