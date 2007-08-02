ifeq ($(TOPDIR),)
    TOPDIR=../../
    INCLUDES=$(TOPDIR)include/
    CPPFLAGS=-g -I$(INCLUDES)
#    CXX=g++
    CXX=/home/artpol/work/openwrt/trunk/openwrt-midge/staging_dir_mipsel/bin/mipsel-linux-uclibc-g++
endif

all: EOC_mr17h.o
	cp EOC_mr17h.o $(TOPDIR)/build

apps: all
	$(CXX) $(CPPFLAGS) -o sender sender.cpp $(TOPDIR)/build/EOC_msg.o EOC_mr17h.o $(TOPDIR)/build/EOC_dev_terminal.o
	$(CXX) $(CPPFLAGS) -o receiver receiver.cpp $(TOPDIR)/build/EOC_msg.o EOC_mr17h.o $(TOPDIR)/build/EOC_dev_terminal.o
	$(CXX) $(CPPFLAGS) -o conf conf.cpp EOC_mr17h.o $(TOPDIR)/build/EOC_dev_terminal.o $(TOPDIR)/build/EOC_msg.o

clean:
	rm -f *.o