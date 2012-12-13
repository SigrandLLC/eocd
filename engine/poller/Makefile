ifeq ($(TOPDIR),)
    TOPDIR=../../
endif

all: EOC_scheduler.o EOC_poller.o EOC_poller_req.o
	cp $^ $(TOPDIR)/build

clean:
	rm -f *.o

