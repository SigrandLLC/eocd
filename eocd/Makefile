#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem U. Polyakov <artpol84@gmail.com>
#	
#***********************************************************************************


TOPDIR=$(PWD)
INCLUDES=$(TOPDIR)/include/
CPPFLAGS=-g -I$(INCLUDES)

.PHONY: generic engine devs

#TARGS :=generic engine devs
#OBJECTS := build/EOC_router.o build/EOC_msg.o build/EOC_scheduler.o build/EOC_dummy.o 
#OBJECTS := build/*.o 


all: $(TARGS) 
	g++ -g -I$(INCLUDES) -o result main.cpp $(OBJECTS)


engine:
	make -C engine/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"
generic:
	make -C generic/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"
devs:
	make -C devs/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"

clean:
	make -C engine clean
	make -C generic clean
	make -C devs clean
	rm -f *.o
	rm -f result
	rm build/*.o
