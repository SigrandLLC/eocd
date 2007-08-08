#***********************************************************************************
#
#	Sigrand EOC management server general Makefile
#	Written 2007 by Artem U. Polyakov <artpol84@gmail.com>
#	
#***********************************************************************************


TOPDIR=$(PWD)
INCLUDES=$(TOPDIR)/include/
CPPFLAGS=-g -I$(INCLUDES)

.PHONY: app-if

TARGS := app-if
CXX=g++
#CXX=/home/artpol/work/openwrt/trunk/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-g++
INCLUDES += -I$(TOPDIR)/libconfig/

all:  
	make lib -C app-if/ INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)

clean:
	make -C app-if clean
