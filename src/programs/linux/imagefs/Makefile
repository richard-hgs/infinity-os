BUILD_DIR=../../../../build/
CURRENT_DIR=programs/linux/$(shell basename $(CURDIR))

CCX=g++
CCXFLAGS=-std=c++14 -W -O -g
LDFLAGS=

SOURCE=$(wildcard *.cpp)
OBJECTS=$(SOURCE:%.cpp=$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o)
TARGET=$(BUILD_DIR)$(CURRENT_DIR)/imagefs.elf

.PHONY: all test
all: $(TARGET)

$(BUILD_DIR)$(CURRENT_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CCX) $(CCXFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	mkdir -p $(dir $@)
	$(CCX) $(CCXFLAGS) -o $@ $^ $(LDFLAGS)

test:
	$(info $$var is [${OBJECTS}])