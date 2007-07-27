ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
endif

INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES)
.PHONY:

all: hash_table.o
	cp $^ $(TOPDIR)/build
#	g++ $(CPPFLAGS) -o result main.cpp hash_table.o

clean:
	rm -f *.o
	rm -f result
