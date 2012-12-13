ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
endif

.PHONY: poller local responder

all: poller local responder

local: EOC_router.o EOC_engine.o EOC_engine_act.o
	cp $^ $(TOPDIR)/build

poller:
	@echo "INCLUDES=$(INCLUDES)"
	make -C poller

responder:
	make -C responder

clean:
	rm -f *.o
	make -C poller clean
	make -C responder clean
