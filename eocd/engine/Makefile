ifeq ($(TOPDIR),)
    TOPDIR=../
endif

INCLUDES=$(TOPDIR)include/

CPPFLAGS=-g -I$(INCLUDES)

all: EOC_router.o
	cp EOC_router.o $(TOPDIR)/build

clean:
	rm -f *.o

