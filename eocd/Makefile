#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem U. Polyakov <artpol84@gmail.com>
#	
#***********************************************************************************


TOPDIR=$(PWD)
INCLUDES=$(TOPDIR)/include/
CPPFLAGS=-g -I$(INCLUDES)

.PHONY: generic engine devs utils db

TARGS :=generic engine devs utils local db
OBJECTS := build/*.o  libconfig++.a 


all: $(TARGS) 
	g++ -g -I$(INCLUDES) -o result main.cpp $(OBJECTS)

local: EOC_main.o
	cp $^ build/    

engine:
	make -C engine/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"
generic:
	make -C generic/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"
devs:
	make -C devs/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"
utils:
	make -C utils/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"
db:
	make -C db/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"

clean:
	make -C engine clean
	make -C generic clean
	make -C devs clean
	rm -f *.o
	rm -f result
	rm -f build/*.o
