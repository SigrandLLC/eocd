ifeq ($(TOPDIR),)
TOPDIR=$(PWD)/../
CPPFLAGS=-g -I$(TOPDIR)/include
endif

all: hash_table.o
	cp $^ $(TOPDIR)/build
	$(CXX) $(CPPFLAGS) -o result test-hash.cpp hash_table.o $(TOPDIR)/build/eoc_debug.o

cont:
	$(CXX) $(CPPFLAGS) -o cont_test EOC_ring_container.cpp
    

clean:
	rm -f *.o
	rm -f result cont_test
