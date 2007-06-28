ifeq ($(TOPDIR),)
    TOPDIR=../../
    INCLUDES=$(TOPDIR)include/
    CPPFLAGS=-g -I$(INCLUDES)
endif

all: EOC_dummy.o
	cp EOC_dummy.o $(TOPDIR)/build

clean:
	rm -f *.o