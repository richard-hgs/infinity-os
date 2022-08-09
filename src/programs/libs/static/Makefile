# BUILD LIBS, KERNEL PROGRAMS, USER PROGRAMS ONE BY ONE
BUILD_DIR=../../../../build/
CURRENT_DIR=programs/libs/$(shell basename $(CURDIR))
LIB_NAME=static

LIBC_SRC_DIR=../../../libs/libc/

INCLUDE_DIRS := -I. -I$(LIBC_SRC_DIR)

C_SOURCES := $(shell find './' -type f -name '*.cpp')
C_OBJECTS := $(patsubst ./%.cpp,$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o, $(C_SOURCES))

COMPILE_OPTIONS=-D__USERMODE
CCX=g++
CCXFLAGS=-static -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
	-fno-pic -std=c++14 -fno-rtti -fno-exceptions -Wall -Wextra -g \
	-ffunction-sections \
	$(COMPILE_OPTIONS) \
	$(INCLUDE_DIRS)

# SYMLINK SCRIPT
# mklink /h "src/programs/libs/static/string.cpp" "src/kernel/stdlibs/string.cpp"
# mklink /h "src/programs/libs/static/string.h" "src/kernel/stdlibs/string.h"
# mklink /h "src/programs/libs/static/stdlib.cpp" "src/kernel/stdlibs/stdlib.cpp"
# mklink /h "src/programs/libs/static/stdlib.h" "src/kernel/stdlibs/stdlib.h"


TARGET=$(BUILD_DIR)$(CURRENT_DIR)/lib$(LIB_NAME).a

.PHONY: all test

all: $(TARGET)

$(TARGET) : $(C_OBJECTS)
	ar --target=elf32-i386 rcs $(TARGET) $(C_OBJECTS)
	objdump -drwC -Mintel $(TARGET) > $(TARGET).dump

# objdump -f $(TARGET) > $(TARGET).elfdump
# objdump -drwC -Mintel $(TARGET) > $(TARGET).dump

$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o : %.cpp
	mkdir -p $(dir $@)
	$(CCX) $(CCXFLAGS) -c -o $@ $<
	objdump -drwC -Mintel $@ > $@.dump

test:
 	$(info $$var is [${C_SOURCES}])