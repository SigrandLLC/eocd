
ifeq ($(TOPDIR),)
    TOPDIR=$(PWD)/../
    CC=gcc
    CXX=g++
    AR=ar
    RANLIB=ranlib
endif

.PHONY:

OBJ_DIR=$(TOPDIR)/build
OBJS= $(OBJ_DIR)/app_comm.o $(OBJ_DIR)/app_frame.o $(OBJ_DIR)/eoc_debug.o
OBJS_CLI=$(OBJ_DIR)/app_comm_cli.o
OBJS_SRV=$(OBJ_DIR)/app_comm_srv.o
OBJS_EXT= $(OBJ_DIR)/eoc_debug.o ../libcomm.a 

all: client server ext_app eoc-info

eoc-info: eoc-info.cpp $(OBJS) $(OBJS_CLI)
	$(CXX) $(CPPFLAGS) -o eoc-info eoc-info.cpp $(OBJS) $(OBJS_CLI)

client: client.cpp $(OBJS) $(OBJS_CLI)
	$(CXX) $(CPPFLAGS) -o cli client.cpp $(OBJS) $(OBJS_CLI)

server: server.cpp $(OBJS) $(OBJS_SRV)
	$(CXX) $(CPPFLAGS) -o srv server.cpp $(OBJS) $(OBJS_SRV)

ext_app: main.c $(OBJS_EXT)
	$(CC) $(CPPFLAGS) -o ext main.c -lstdc++ $(OBJS_EXT)


clean:
	rm -f *.o *.lo
	rm -f ext cli srv