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

all: client server eoc-info eoc-config

eoc-info: eoc-info.cpp $(OBJS) $(OBJS_CLI)
	$(CXX) $(CPPFLAGS) -o eoc-info app-utils.cpp app-utils-jason.cpp eoc-info.cpp $(OBJS) $(OBJS_CLI)
#	$(CXX) $(CPPFLAGS) -o test app-utils.cpp test.cpp $(OBJS) $(OBJS_CLI)
#	$(CXX) $(CPPFLAGS) -o test1 app-utils.cpp test1.cpp $(OBJS) $(OBJS_CLI)

eoc-config: eoc-config.cpp $(OBJS) $(OBJS_CLI)
	$(CXX) $(CPPFLAGS) -o eoc-config app-utils.cpp eoc-config.cpp $(OBJS) $(OBJS_CLI)

client: client.cpp $(OBJS) $(OBJS_CLI)
	$(CXX) $(CPPFLAGS) -o cli client.cpp $(OBJS) $(OBJS_CLI)

server: server.cpp $(OBJS) $(OBJS_SRV)
	$(CXX) $(CPPFLAGS) -o srv server.cpp $(OBJS) $(OBJS_SRV)

clean:
	rm -f *.o *.lo
	rm -f ext cli srv eoc-info eoc-config test test1
