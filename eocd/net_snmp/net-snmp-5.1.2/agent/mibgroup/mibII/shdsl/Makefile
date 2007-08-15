#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem U. Polyakov <artpol84@gmail.com>
#	
#***********************************************************************************


#LOC_PWD=$(PWD)
TOPDIR=$(PWD)/eocd
INCLUDES=$(TOPDIR)/include/
CPPFLAGS=-g -I$(INCLUDES)

.PHONY: app-if

TARGS := app-if
CXX=g++
#CXX=/home/artpol/work/openwrt/trunk/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-g++
INCLUDES += -I$(TOPDIR)/libconfig/

all:
	@echo "TOPDIR="$(TOPDIR)  
	make lib -C eocd/app-if/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)
	mkdir -p ../../../.libs
	cp eocd/app-if/libcomm.a ../../../.libs

clean:

installlibs:
install: