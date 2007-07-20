ifeq ($(TOPDIR),)
    TOPDIR=../
endif

INCLUDES=$(TOPDIR)include/

CPPFLAGS=-g -I$(INCLUDES)

all: EOC_db.o
	cp EOC_db.o $(TOPDIR)/build


clean:
	rm -f *.o

