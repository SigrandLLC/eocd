
ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
    CC=gcc
    CXX=g++
    AR=ar
    RANLIB=ranlib
endif

.PHONY: install lib objects apps

all: lib install

objects: app_comm.o app_comm_cli.o app_comm_srv.o app_frame.o comm.o

install: app_comm.o app_comm_cli.o app_comm_srv.o app_frame.o
	cp $^ $(TOPDIR)/build

all: install apps
#	g++ $(CPPFLAGS) -o result main.cpp

apps: install
	make -C applications

lib: objects
	$(AR) rc libcomm.a app_comm.o app_comm_cli.o app_comm_srv.o app_frame.o comm.o
	$(RANLIB) libcomm.a

clean:
	rm -f *.o *.lo
	rm -f result libcomm.* ext cli srv
	make -C applications clean