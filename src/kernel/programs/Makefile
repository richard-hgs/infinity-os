# BUILD LIBS, KERNEL PROGRAMS, USER PROGRAMS ONE BY ONE
CURRENT_DIR=$(shell basename $(CURDIR))

.PHONY: all test

all:
	cd $(CURDIR)/libs && $(MAKE)
	cd $(CURDIR)/user/shell && $(MAKE)

test:
 	$(info $$var is [${PROJECT_DIR}])