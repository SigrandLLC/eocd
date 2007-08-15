
ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
    CC=gcc
    CXX=g++
    AR=ar
    RANLIB=ranlib
endif

INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES) -I./

.PHONY: lib first objects

first:
	@echo "TOPDIR="$(TOPDIR)
	@echo "INCLUDES="$(INCLUDES)	
	@echo "CPPFLAGS="$(CPPFLAGS)	


objects: app_comm.o app_comm_cli.o app_comm_srv.o app_frame.o comm.o
	$(CXX) $(CPPFLAGS) -c -fPIC -o app_comm.lo app_comm.cpp
	$(CXX) $(CPPFLAGS) -c -fPIC -o app_comm_cli.lo app_comm_cli.cpp
	$(CXX) $(CPPFLAGS) -c -fPIC -o app_comm_srv.lo app_comm_srv.cpp
	$(CXX) $(CPPFLAGS) -c -fPIC -o app_frame.lo app_frame.cpp
	$(CXX) $(CPPFLAGS) -c -fPIC -o comm.lo comm.cpp


install: app_comm.o app_comm_cli.o app_comm_srv.o app_frame.o
	cp $^ $(TOPDIR)/build

all: install client server ext_app
#	g++ $(CPPFLAGS) -o result main.cpp

client: all
	$(CXX) $(CPPFLAGS) -o cli client.cpp app_comm_cli.o app_comm.o app_frame.o
server: all
	$(CXX) $(CPPFLAGS) -o srv server.cpp app_comm_srv.o app_comm.o app_frame.o

lib: objects
	$(AR) rc libcomm.a app_comm.o app_comm_cli.o app_comm_srv.o app_frame.o comm.o
	$(RANLIB) libcomm.a
	$(CXX) $(CPPFLAGS) -shared -fPIC app_comm_srv.lo app_comm.lo app_frame.lo comm.lo -o libcomm.so


ext_app: lib
	$(CC) $(CPPFLAGS) -o ext main.c libcomm.a -lstdc++


clean:
	rm -f *.o *.lo
	rm -f result libcomm.* ext cli srv