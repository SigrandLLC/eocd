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
    enum { BLOCK_SIZE = 256 };

    char *sname;
    int sfd;
    fd_set socks;
    int hisock;    
    
    int set_nonblock(int sock);
    virtual void build_select_list();
    virtual int complete_wait() = 0;

    // data transparency && message end/begin flags
    char *transp(char *buf,size_t size,size_t &nsize);
    char *untransp(char *frame,size_t frame_len);
    
public:
    app_comm(char *sock_name){
	sname = strndup(sock_name,MAX_SOCK_NAME);
    }
    app_comm(char *sock_path,char *sock_name){
	int len = strnlen(sock_path,MAX_SOCK_NAME) + strnlen(sock_name,MAX_SOCK_NAME);
	sname = (char*)malloc(sizeof(char) * len);
	snprintf(sname,len,"%s/%s",sock_path,sock_name);
    }
    int wait();
    int _send(int fd,char *buf,size_t size);
    ssize_t _recv(int fd,char *&buf);
    virtual int send(int conn_num,char *buf,size_t size) = 0;
    virtual ssize_t recv(int &conn_num,char *buf) = 0;
};

#endif


