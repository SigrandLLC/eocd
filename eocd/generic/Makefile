@echo "TOPDIR=$(TOPDIR)"
ifeq ($(TOPDIR),)
    TOPDIR=../
    INCLUDES=$(TOPDIR)include/
    CPPFLAGS=-g -I$(INCLUDES)
endif


all: EOC_msg.o
	cp EOC_msg.o $(TOPDIR)/build

clean:
	rm -f *.o

