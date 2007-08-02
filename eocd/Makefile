#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem U. Polyakov <artpol84@gmail.com>
#	
#***********************************************************************************


TOPDIR=$(PWD)
INCLUDES=$(TOPDIR)/include/
CPPFLAGS=-g -I$(INCLUDES)

.PHONY: generic engine devs utils db app-if

TARGS :=generic engine devs utils local db app-if
OBJECTS := build/*.o 
#CXX=g++
CXX=/home/artpol/work/openwrt/trunk/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-g++
INCLUDES += -I$(TOPDIR)/libconfig/

all: $(TARGS) 
	$(CXX) -g -I$(INCLUDES) -o result main.cpp $(OBJECTS) libconfig++.a

all-mips: $(TARGS) 
	$(CXX) -g -I$(INCLUDES) -o result main.cpp $(OBJECTS) libconfig/.libs/libconfig++.a

local: EOC_main.o
	cp $^ build/    

engine:
	make -C engine/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)
generic:
	make -C generic/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)
devs:
	make -C devs/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)
utils:
	make -C utils/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)
db:
	make -C db/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)
app-if:
	make client server -C app-if/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)


clean:
	make -C engine clean
	make -C generic clean
	make -C devs clean
	make -C utils clean
	make -C db clean
	make -C app-if clean
	rm -f *.o
	rm -f result
	rm -f build/*.o
