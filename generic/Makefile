ifeq ($(TOPDIR),)
    TOPDIR=../
    INCLUDES=$(TOPDIR)include/
    CPPFLAGS=-g -I$(INCLUDES)
endif


all: EOC_msg.o eoc_debug.o
	cp $^ $(TOPDIR)/build

clean:
	rm -f *.o

