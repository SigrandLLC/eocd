ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
endif

INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES)
.PHONY: poller local responder db

all: poller local responder db

local: EOC_router.o EOC_engine.o
	cp $^ $(TOPDIR)/build

poller:
	make -C poller TOPDIR=$(TOPDIR)

responder:
	make -C responder TOPDIR=$(TOPDIR)

db:
	make -C db TOPDIR=$(TOPDIR)

clean:
	rm -f *.o
	make -C poller clean
	make -C responder clean
	make -C db clean