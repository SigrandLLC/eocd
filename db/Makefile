ifeq ($(TOPDIR),)
    TOPDIR=../
endif

all: EOC_db.o EOC_loop.o
	cp $^ $(TOPDIR)/build
#	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o main main.cpp EOC_loop.o

clean:
	rm -f *.o
	rm -Rf main

