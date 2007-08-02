.PHONY: mr17h dummy generic

all: mr17h dummy generic

mr17h:
	make -C MR17H INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)

dummy:
	make -C dummy INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)

generic:
	make -C generic INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)" CXX=$(CXX)

clean:
	make -C MR17H clean
	make -C dummy clean	
	make -C generic clean

