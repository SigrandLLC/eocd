.PHONY: mr17h dummy

all: mr17h dummy

mr17h:
	make -C MR17H INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"

dummy:
	make -C dummy INCLUDES=$(INCLUDES) TOPDIR=$(TOPDIR) CPPFLAGS="$(CPPFLAGS)"

clean:
	make -C MR17H clean
	make -C dummy clean	
