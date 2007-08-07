ifeq ($(TOPDIR),)
    TOPDIR=../
endif

INCLUDES=$(TOPDIR)include/

CPPFLAGS=-g -I$(INCLUDES)

all: EOC_db.o EOC_loop.o
	cp $^ $(TOPDIR)/build
	$(CXX) $(CPPFLAGS) -o main main.cpp EOC_loop.o

clean:
	rm -f *.o

