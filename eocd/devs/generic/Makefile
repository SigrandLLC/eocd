ifeq ($(TOPDIR),)
    TOPDIR=../../
    INCLUDES=$(TOPDIR)include/
    CPPFLAGS=-g -I$(INCLUDES)
endif

all: EOC_dev_terminal.o
	cp $? $(TOPDIR)/build

clean:
	rm -f *.o