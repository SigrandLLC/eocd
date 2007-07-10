ifeq ($(TOPDIR),)
    TOPDIR=../../
endif
INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES)

all: EOC_resp_handlers.o
	cp $? $(TOPDIR)/build


clean:
	rm -f *.o

