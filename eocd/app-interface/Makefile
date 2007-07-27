ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
endif

INCLUDES=$(TOPDIR)include/
CPPFLAGS=-g -I$(INCLUDES) -I./
.PHONY:

all: app_comm.o app_comm_cli.o app_comm_srv.o
#	gcc $(CPPFLAGS) -o cli client.c unix-interface.c
#	gcc $(CPPFLAGS) -o srv server.c unix-interface.c

clean:
	rm -f *.o
	rm -f result
