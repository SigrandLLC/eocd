#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem Y. Polyakov <artpol84@gmail.com>
#
#***********************************************************************************


TOPDIR=$(PWD)

include $(TOPDIR)/version.mk

INCLUDES=-I$(TOPDIR)/include/
INCLUDES += -I$(TOPDIR)/libconfig/
CPPFLAGS=$(INCLUDES) -DEOC_VER="\"$(EOC_VER)\""
#-g
ifeq ($(VIRT),1)
CPPFLAGS += -DVIRTUAL_DEVS
endif


.PHONY: generic engine devs utils db app-if tar

TARGS :=generic engine devs utils local db app-if
OBJECTS := build/*.o

#ifneq ($(VIRT),1)
CC     ?= $(CROSS_PREFIX)gcc
CXX    ?= $(CROSS_PREFIX)g++
AR     ?= $(CROSS_PREFIX)ar
RANLIB ?= $(CROSS_PREFIX)ranlib
#else
#RANLIB=ranlib
#endif

export CXX AR RANLIB CC TOPDIR INCLUDES CPPFLAGS


all: $(TARGS)
	$(CXX) -g $(INCLUDES) -o result main.cpp $(OBJECTS) libconfig++.a

md5.o : md5.c
	$(CC)  $(CPPFLAGS) $(CFLAGS)         -c md5.c

kdb.o : kdb.c
	$(CC)  $(CPPFLAGS) $(CFLAGS) -DSHELL -c kdb.c

all-mips: $(TARGS) md5.o kdb.o
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o eocd eocd.cpp kdb.o md5.o $(OBJECTS) libconfig/.libs/libconfig++.a

fake-mips: $(TARGS)
#	$(CXX) -g $(INCLUDES) -o result main.cpp $(OBJECTS) libconfig/.libs/libconfig++.a
	$(CXX) -g $(INCLUDES) -o eocd-fake eocd-fake.cpp $(OBJECTS) libconfig/.libs/libconfig++.a

memleak-test:
	$(CXX) -g $(INCLUDES) -o memleak-test memleak-test.cpp kdb.o md5.o $(OBJECTS)

local: EOC_main.o EOC_pci.o
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
	rm -f result eocd eocd-fake memleak-test
	rm -f build/*.o


ARCH_FILES=app-if build CHECK db devs engine eocd.conf.default eocd.cpp EOC_main.cpp EOC_pci.cpp \
generic include main.cpp Makefile.tmpl system TODO.txt utils version.mk Changelog kdb.c kdb.h md5.c md5.h

tar: clean
	tar --exclude="\.svn" -cjvf tar/eocd-$(EOC_VER).tar.bz2 $(ARCH_FILES)

