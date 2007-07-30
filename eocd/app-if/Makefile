ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
endif

INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES) -I./
.PHONY:

all: app_comm.o app_comm_cli.o app_comm_srv.o
	cp $^ $(TOPDIR)/build
#	g++ $(CPPFLAGS) -o result main.cpp
#	g++ $(CPPFLAGS) -o cli client.cpp app_comm_cli.o app_comm.o
#	g++ $(CPPFLAGS) -o srv server.cpp app_comm_srv.o app_comm.o

clean:
	rm -f *.o
	rm -f result
	rm cli srv result