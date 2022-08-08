# BUILD LIBS, KERNEL PROGRAMS, USER PROGRAMS ONE BY ONE
CURRENT_DIR=$(shell basename $(CURDIR))

.PHONY: all test

all:
    cd $(CURDIR)/static && $(MAKE)
	cd $(CURDIR)/kernel && $(MAKE)
	cd $(CURDIR)/user && $(MAKE)

test:
 	$(info $$var is [${CURRENT_DIR}])