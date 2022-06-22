# Simple makefile for gcc written by stext editor.
BUILD_DIR=../../../build/
CURRENT_DIR=$(shell basename $(CURDIR))

CC=gcc
CFLAGS=-std=c11 -W -O -g
LDFLAGS=

BACKUPS=$(shell find . -iname "*.bak")
SRCDIR=$(shell basename $(shell pwd))
DESTDIR?=
PREFIX?=usr/local
VERSION=1.0

SOURCE=$(wildcard *.c)
OBJECTS=$(SOURCE:%.c=$(BUILD_DIR)$(CURRENT_DIR)/%.c.o)
TARGET=$(BUILD_DIR)$(CURRENT_DIR)/imgwrite

.PHONY: all clean distclean
all: $(TARGET)

$(BUILD_DIR)$(CURRENT_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(TARGET)

distclean: clean
ifneq ($(BACKUPS),)
	rm -f *.bak
endif
