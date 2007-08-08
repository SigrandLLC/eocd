ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
    CC=gcc
    CXX=g++
endif

INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES) -I./
.PHONY: lib

all: app_comm.o app_comm_cli.o app_comm_srv.o app_frame.o
	cp $^ $(TOPDIR)/build
#	g++ $(CPPFLAGS) -o result main.cpp
client: all
	$(CXX) $(CPPFLAGS) -o cli client.cpp app_comm_cli.o app_comm.o app_frame.o
server: all
	$(CXX) $(CPPFLAGS) -o srv server.cpp app_comm_srv.o app_comm.o app_frame.o

lib: app_comm.o app_comm_cli.o app_frame.o comm.o
	ar rc libcomm.a $^

ext_app: lib
	$(CC) $(CPPFLAGS) -o ext main.c libcomm.a -lstdc++

clean:
	rm -f *.o
	rm -f result libcomm.a ext cli srv