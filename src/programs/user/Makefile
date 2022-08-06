# BUILD LIBS, KERNEL PROGRAMS, USER PROGRAMS ONE BY ONE
CURRENT_DIR=$(shell basename $(CURDIR))

.PHONY: all test

all:
	cd $(CURDIR)/shell && $(MAKE)
	cd $(CURDIR)/fs && $(MAKE)

test:
 	$(info $$var is [${CURRENT_DIR}])