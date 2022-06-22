# =============================================================
# Simple Makefile for imagefs
# Created by Infinity Technology
# =============================================================
BUILD_DIR=../../../build/
CURRENT_DIR=$(shell basename $(CURDIR))

CC=gcc
CFLAGS=-std=c89 -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS=

TARGET=$(BUILD_DIR)$(CURRENT_DIR)/imagefs
SOURCE=$(wildcard *.c)
OBJECT=$(SOURCE:%.c=$(BUILD_DIR)$(CURRENT_DIR)/%.c.o)

BACKUPS=$(shell find . -iname "*.bak")

.PHONY: all test clean distclean
all: $(TARGET)

test:
	$(info $$var is [${OBJECT}])

$(BUILD_DIR)$(CURRENT_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	@echo CC $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECT)
	@echo CCLD $(CFLAGS) $(LDFLAGS) -o $@ $^
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f $(OBJECT) $(TARGET)

distclean:
ifneq ($(BACKUPS),)
	rm -f $(BACKUPS)
endif
