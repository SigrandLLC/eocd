#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem U. Polyakov <artpol84@gmail.com>
#	
#***********************************************************************************


#LOC_PWD=$(PWD)
TOPDIR=$(PWD)
INCLUDES=$(TOPDIR)/include/
INCLUDES += -I$(TOPDIR)/libconfig/
CPPFLAGS=-g -I$(INCLUDES)

.PHONY: app-if

TARGS := app-if

ifeq ($(RANLIB),)
    RANLIB := ranlib
    export RANLIB
endif


export CPPFLAGS

all:
	@echo "CXX=$(CXX)"
	make lib -C app-if/
	mkdir -p ../../../.libs
#	cp app-if/libcomm.a ../../../.libs

clean:

installlibs:
install: