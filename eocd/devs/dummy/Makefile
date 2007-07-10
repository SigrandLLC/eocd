ifeq ($(TOPDIR),)
    TOPDIR=../../
    INCLUDES=$(TOPDIR)include/
    CPPFLAGS=-g -I$(INCLUDES)
endif

all: EOC_dummy.o EOC_dummy1.o
	cp $? $(TOPDIR)/build

clean:
	rm -f *.o