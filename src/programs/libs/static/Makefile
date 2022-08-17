# BUILD LIBS, KERNEL PROGRAMS, USER PROGRAMS ONE BY ONE
BUILD_DIR=../../../../build/
CURRENT_DIR=programs/libs/$(shell basename $(CURDIR))
LIB_NAME=static
BUILD_DIR_INCLUDE=$(BUILD_DIR)$(CURRENT_DIR)/include

LIBC_SRC_DIR=../../../libs/libc/

INCLUDE_DIRS := -I. -Idrivers -Istdlibs -Imemory -I$(LIBC_SRC_DIR)

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
# mklink /h "src/programs/libs/static/stdlibs/string.cpp" "src/kernel/stdlibs/string.cpp"
# mklink /h "src/programs/libs/static/stdlibs/string.h" "src/kernel/stdlibs/string.h"
# mklink /h "src/programs/libs/static/stdlibs/stdlib.cpp" "src/kernel/stdlibs/stdlib.cpp"
# mklink /h "src/programs/libs/static/stdlibs/stdlib.h" "src/kernel/stdlibs/stdlib.h"
# mklink /h "src/programs/libs/static/drivers/fat.h" "src/kernel/drivers/fat.h"
# mklink /h "src/programs/libs/static/drivers/fat.cpp" "src/kernel/drivers/fat.cpp"
# mklink /h "src/programs/libs/static/memory/memutils.h" "src/kernel/memory/memutils.h"
# mklink /h "src/programs/libs/static/memory/memutils.cpp" "src/kernel/memory/memutils.cpp"


TARGET=$(BUILD_DIR)$(CURRENT_DIR)/lib$(LIB_NAME).a

.PHONY: all test

all: $(TARGET)

$(TARGET) : $(C_OBJECTS)
	ar --target=elf32-i386 rcs $(TARGET) $(C_OBJECTS)
	objdump -drwC -Mintel $(TARGET) > $(TARGET).dump

# objdump -f $(TARGET) > $(TARGET).elfdump
# objdump -drwC -Mintel $(TARGET) > $(TARGET).dump

$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o : %.cpp
	mkdir -p $(BUILD_DIR_INCLUDE)
	mkdir -p $(dir $@)
	$(CCX) $(CCXFLAGS) -c -o $@ $<
	cp $(patsubst %.cpp,%.h, $<) $(BUILD_DIR_INCLUDE)/$(patsubst %.cpp,%.h, $(shell basename $<))
	objdump -drwC -Mintel $@ > $@.dump
	

test:
 	$(info $$var is [${C_SOURCES}])