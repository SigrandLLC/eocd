ifeq ($(TOPDIR),)
    TOPDIR=../../
endif

INCLUDES=$(TOPDIR)include/

CPPFLAGS=-g -I$(INCLUDES)

all: EOC_scheduler.o EOC_poller.o EOC_poller_req.o
	cp $^ $(TOPDIR)/build


clean:
	rm -f *.o

