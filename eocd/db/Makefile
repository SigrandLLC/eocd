ifeq ($(TOPDIR),)
    TOPDIR=../
endif

INCLUDES=$(TOPDIR)include/

CPPFLAGS=-g -I$(INCLUDES)

all: EOC_db.o EOC_loop.o
	cp $^ $(TOPDIR)/build


clean:
	rm -f *.o

