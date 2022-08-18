# BUILD LIBS, KERNEL PROGRAMS, USER PROGRAMS ONE BY ONE
CURRENT_DIR=$(shell basename $(CURDIR))

.PHONY: all test

all:
	cd $(CURDIR)/libs && $(MAKE)
	cd $(CURDIR)/kernel && $(MAKE)
	cd $(CURDIR)/user && $(MAKE)
	cd $(CURDIR)/linux/imagefs && $(MAKE)

test:
 	$(info $$var is [${CURRENT_DIR}])