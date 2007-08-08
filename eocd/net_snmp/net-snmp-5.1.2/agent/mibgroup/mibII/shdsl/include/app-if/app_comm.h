#ifndef EOCD_APP_COMMUNICATOR_H
#define EOCD_APP_COMMUNICATOR_H

#include <sys/types.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <eocd_log.h>



class app_comm{
protected:
    enum { MAX_SOCK_NAME = 256 };
    enum { BLOCK_SIZE = 1024 };
    
    int error_init;

    char *sname;
    int sfd;
    fd_set socks;
    int hisock;    
    
    int init_success(){ return !error_init; }
    int set_nonblock(int sock);
    virtual int build_select_list();
    virtual int complete_wait() = 0;

    // data transparency && message end/begin flags
    char *transp(char *buf,size_t size,size_t &nsize){
	nsize = size;
	return buf;
    }
    char *untransp(char *frame,size_t frame_len){
	return frame;
    }
public:
    app_comm(char *sock_name){
	sname = strndup(sock_name,MAX_SOCK_NAME);
	error_init = 0;
    }
    app_comm(char *sock_path,char *sock_name){
	int len = strnlen(sock_path,MAX_SOCK_NAME) + strnlen(sock_name,MAX_SOCK_NAME) + 2;
	sname = (char*)malloc(sizeof(char) * len);
	snprintf(sname,len,"%s/%s",sock_path,sock_name);
	error_init = 0;
    }
    int wait();
    int _send(int fd,char *buf,size_t size);
    ssize_t _recv(int fd,char *&buf);
    int init_ok(){ return (!error_init); }
};

#endif


