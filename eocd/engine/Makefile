ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
endif

INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES)
.PHONY: poller local responder

all: poller local responder

local: EOC_router.o EOC_engine.o
	cp $^ $(TOPDIR)/build

poller:
	make -C poller TOPDIR=$(TOPDIR)

responder:
	make -C responder TOPDIR=$(TOPDIR)


clean:
	rm -f *.o
	make -C poller clean
	make -C responder clean
