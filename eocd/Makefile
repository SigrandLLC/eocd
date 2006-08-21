#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem U. Polyakov <artpol84@gmail.com>
#	
#***********************************************************************************


TOPDIR=$(PWD)
INCLUDES=-I$(TOPDIR)/include/
INCLUDES += -I$(TOPDIR)/libconfig/
CPPFLAGS=-g $(INCLUDES)

.PHONY: generic engine devs utils db app-if tar

TARGS :=generic engine devs utils local db app-if
OBJECTS := build/*.o 

CC=/home/artpol/work/openwrt/trunk/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-gcc
CXX=/home/artpol/work/openwrt/trunk/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-g++
AR=/home/artpol/work/openwrt/trunk/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-ar
RANLIB=/home/artpol/work/openwrt/trunk/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-ranlib

#RANLIB=ranlib

export CXX AR RANLIB CC TOPDIR INCLUDES CPPFLAGS


all: $(TARGS) 
	$(CXX) -g $(INCLUDES) -o result main.cpp $(OBJECTS) libconfig++.a
#	$(CXX) -g $(INCLUDES) -o eocd eocd.cpp $(OBJECTS) libconfig++.a

all-mips: $(TARGS) 
	$(CXX) -g $(INCLUDES) -o result main.cpp $(OBJECTS) libconfig/.libs/libconfig++.a
	$(CXX) -g $(INCLUDES) -o eocd eocd.cpp $(OBJECTS) libconfig/.libs/libconfig++.a

local: EOC_main.o
	cp $^ build/    

engine:
	make all -C engine/ 
generic:
	make all -C generic/ 
devs:
	make all -C devs/ 
utils:
	make all -C utils/ 
db:
	make all -C db/ 
app-if:
	make all -C app-if/ 


clean:
	make -C engine clean
	make -C generic clean
	make -C devs clean
	make -C utils clean
	make -C db clean
	make -C app-if clean
	rm -f *.o
	rm -f result eocd
	rm -f build/*.o


ARCH_FILES=app-if build CHECK db devs engine eocd.conf eocd.cpp EOC_main.cpp \
generic include main.cpp Makefile.tmpl system TODO.txt utils

tar: clean
	tar --exclude="\.svn" -cjvf tar/eocd.tar.bz2 $(ARCH_FILES)
