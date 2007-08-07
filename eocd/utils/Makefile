ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
endif

INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES)
.PHONY:

all: hash_table.o
	cp $^ $(TOPDIR)/build
	$(CXX) $(CPPFLAGS) -o result test-hash.cpp hash_table.o

cont:
	$(CXX) $(CPPFLAGS) -o cont_test EOC_ring_container.cpp
    

clean:
	rm -f *.o
	rm -f result
