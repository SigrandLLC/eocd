ifeq ($(TOPDIR),)
    TOPDIR=../../
    INCLUDES=$(TOPDIR)include/
    CPPFLAGS=-g -I$(INCLUDES)
endif

all: EOC_sg17.o
	cp EOC_sg17.o $(TOPDIR)/build

clean:
	rm -f *.o