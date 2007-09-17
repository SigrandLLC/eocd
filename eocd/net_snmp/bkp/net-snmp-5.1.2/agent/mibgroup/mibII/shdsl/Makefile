#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem U. Polyakov <artpol84@gmail.com>
#	
#***********************************************************************************


#LOC_PWD=$(PWD)
TOPDIR=$(PWD)
INCLUDES=$(TOPDIR)/include/
CPPFLAGS=-g -I$(INCLUDES)

export CXX CPPFLAGS TOPDIR 

.PHONY: app-if

TARGS := app-if
CXX=g++

ifeq ($(RANLIB),)
    RANLIB := ranlib
    export RANLIB
endif

INCLUDES += -I$(TOPDIR)/libconfig/



all:
	make lib -C app-if/
	mkdir -p ../../../.libs
	cp app-if/libcomm.a ../../../.libs

clean:

installlibs:
install: