#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "include/unix_interface.h"
#include "include/eocd_log.h"

/* TODO:
	1. Channel syncronysation!
(+)	2. Determine that socket is closed ??
*/


//---- Socket service functions ------------//

static int
_set_nonblock(int sock)
{
	int opts;

	opts = fcntl(sock,F_GETFL);
	if (opts < 0) {
		eocd_perror("fcntl(F_GETFL)");
		return -errno;
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock,F_SETFL,opts) < 0) {
		eocd_perror("fcntl(F_SETFL)");
		return -errno;
	}
	return 0;
}

//---- Sockets service functions ----//

/*
 _build_select_list:
	Build list of file descriptors for select from eocd descriptor d
*/
static void
_build_select_list(Eocd_desc *d)
{
	int i;
	// blank fd set
	FD_ZERO(&d->socks);
	// fill fd set
	d->hisock = d->fd[0];
	for(i=0; i < d->fd_num; i++) {
		FD_SET(d->fd[i],&d->socks);
		if (d->fd[i] > d->hisock)
			d->hisock = d->fd[i];
		
	}
}

static int
_new_connection(Eocd_desc *d)
{
	int i,ret=0;
	int nsock; /* Socket file descriptor for incoming connections */

	nsock = accept(d->fd[0], NULL, NULL);
	if( nsock < 0) {
		eocd_perror("accept");
		return -errno;
	}
	if( (ret = _set_nonblock(nsock)) ){
		close(nsock);
		return ret;
	}

	if( d->fd_num == (MAX_CONNECTIONS + 1) ){
		close(nsock);
		eocd_log(WARNING,"Close new connection - no room left");
		return -ENOMEM;
	}
	// add to socket list
	d->fd[d->fd_num++] = nsock;
	return 0;
}

static int
_complete_srv_wait(Eocd_desc *d)
{
    int i,j;
    int num_act=0;
    char tmp;
	
    memset(d->fd_act,0,sizeof(d->fd_act));	
    for (i=1; i<d->fd_num; i++) {
	if (FD_ISSET(d->fd[i],&d->socks)){
	    if( recv(d->fd[i],&tmp,1,MSG_PEEK|MSG_DONTWAIT) < 1 ){
		eocd_log(WARNING,"Connection closed");
		close(d->fd[i]);
		// shift descriptors
		for(j=i;j<d->fd_num-1;j++)
		    d->fd[j]=d->fd[j+1];
		d->fd_num--;
		i--;
	    }else{
		d->fd_act[i] = 1;
		num_act++;
	    }
	}
    }
    if (FD_ISSET(d->fd[0],&d->socks))
	_new_connection(d);
    return num_act;
}

static int
_complete_cli_wait(Eocd_desc *d)
{
    if (FD_ISSET(d->fd[0],&d->socks))
	return 1;
    return 0;
}

static int
_eocd_wait(Eocd_desc *d)
{
    int count=0;
    struct timeval timeout;  /* Timeout for select */	
	
    while(1){
        timeout.tv_sec = 1;
	timeout.tv_usec = 0;
    	_build_select_list(d);

	count = select(d->hisock+1,&d->socks,(fd_set *)0,(fd_set *)0, &timeout);
	if( count < 0) {
	    eocd_perror("Select");
	    return -errno;
	}
	if( count ){
	    switch( d->type ){
	    case EOCD_SERVER:
		return _complete_srv_wait(d);
	    case EOCD_CLIENT:
		return _complete_cli_wait(d);	    
	    }
	}
    }
}

inline Eocd_desc *
eocd_alloc_desc(){
    Eocd_desc *d = (Eocd_desc *)malloc(sizeof(Eocd_desc));
    memset(d,0,sizeof(Eocd_desc));
    return d;
}

inline void
eocd_free_desc(Eocd_desc *d){
    free(d);
}

int
eocd_init_client(Eocd_desc *desc)
{
    struct sockaddr_un saun;
    struct stat sbuf;    
    int s;
    int ret=0,len;

    // Check path exist
    if( (ret = stat(SOCKET_PATH SOCKET_NAME,&sbuf)) ){
	if( errno != ENOENT ){
	    eocd_perror("Error getting info about %s",SOCKET_PATH);
	    return -errno;
	}
    }  
    if( !S_ISSOCK(sbuf.st_mode) ){
	eocd_log(ERROR,"No socket (%s)",SOCKET_PATH SOCKET_NAME);
	return -1;
    }

    // Create socket
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    	eocd_perror("Cannot create socket");
	return -errno;    
    }

    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path, SOCKET_PATH SOCKET_NAME);
	
    len = sizeof(saun.sun_family) + strlen(saun.sun_path);
    if (connect(s,(struct sockaddr*)&saun, len) < 0) {
	eocd_perror("Cannot connect to %s", SOCKET_PATH SOCKET_NAME);
	return -errno;
    }
    desc->type=EOCD_CLIENT;
    desc->fd[0] = s;
    desc->fd_num = 1;
    return 0;
}

int
eocd_init_server(Eocd_desc *desc)
{
    struct sockaddr_un saun;
    struct stat sbuf;
    int s;
    int ret=0,len;
    
    // Check path exist
    if( (ret = stat(SOCKET_PATH,&sbuf)) ){
	if( errno != ENOENT ){
	    eocd_perror("Error getting info about %s",SOCKET_PATH);
	    return -errno;
	}
	if( mkdir(SOCKET_PATH,(S_IRWXU | S_IRGRP | S_IXGRP)) ){
	    eocd_perror("Cannot create dir %s",SOCKET_PATH);
	    return -errno;
	}
	if( stat(SOCKET_PATH,&sbuf) ){
	    eocd_perror("Error creating dir %s",SOCKET_PATH);
	    return -errno;
	}
    }  
    if( !S_ISDIR(sbuf.st_mode) ){
	eocd_log(ERROR,"Error: %s is not directory",SOCKET_PATH);
	return -1;
    }

    // Create socket
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    	eocd_perror("Cannot create socket");
	return -errno;    
    }
			    
    if( unlink(SOCKET_PATH SOCKET_NAME) ){
	if( errno != ENOENT ){
	    eocd_perror("Cannot unlink %s",SOCKET_PATH SOCKET_NAME);
	    return -errno;
	}
    }		
    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path,SOCKET_PATH SOCKET_NAME);
    len = sizeof(saun.sun_family) + strlen(saun.sun_path);
    if(bind(s, &saun, len) < 0) {
    	eocd_perror("Cannot bind server");
	return -errno;
    }
    // open socket for listening
    if(listen(s,MAX_CONNECTIONS) < 0) {
    	eocd_perror("Cannot bind server");
	return -errno;
    }
    // setup non blocking
    if( _set_nonblock(s) )
/*??*/    return -errno;
    // setup descriptor
    desc->type=EOCD_SERVER;
    desc->fd[0] = s;
    desc->fd_num = 1;

    return 0;			
}

inline int
eocd_wait(Eocd_desc *desc){
    return _eocd_wait(desc);
}

inline int
eocd_nex_fd(Eocd_desc *d){
    int i;
    for(i=0;i<d->fd_num;i++){
	if( d->fd_act[i] ){
	    d->fd_act[i] = 0;
	    return d->fd[i];
	}
    }
    return -1;
}

//---- Message flow control functions ----// 

Eocd_msg *
eocd_alloc_msg(int len)
{
    Eocd_msg *_msg;
    _msg = malloc(sizeof(*_msg) + sizeof(char)*len);
    if( !_msg )
	return NULL;
    _msg->len = len + sizeof(*_msg);
    return _msg;	
}

int
eocd_setup_msg(Eocd_msg *msg,int id,int sub_id,int span,char dev)
{
    msg->id = id;
    msg->sub_id = sub_id;
    msg->span = span;
    msg->dev = dev;
    return 0;
}

char*
eocd_get_dataptr(Eocd_msg *msg)
{
    return ((char*)msg + sizeof(*msg));
}


inline void
eocd_set_msg_status(Eocd_msg *msg,Eocd_msg_status status){
    msg->status = status;
}

Eocd_msg_status
eocd_get_msg_status(Eocd_msg *msg)
{
    switch( msg->status ){
    case 0:
	return REQUEST;
    case 1:
	return RESPONSE;
    }
}


int
eocd_send_msg(int fd,Eocd_msg *msg)
{

    if( send(fd,(char*)msg,msg->len,0) != msg->len )
	return -EAGAIN;
}	

int
eocd_recv_msg(int fd,Eocd_msg **msg)
{
    Eocd_msg *_msg;
    int msg_len;
    int ret;
    
    *msg = NULL;
    
    if( recv(fd,&msg_len,sizeof(msg_len),MSG_PEEK|MSG_DONTWAIT) < sizeof(int) )
    	return -EAGAIN;
    
    _msg = (Eocd_msg *)malloc(msg_len * sizeof(char));
    if( !_msg )
	return -ENOMEM;
    
    ret = recv(fd,(char*)_msg,msg_len,MSG_DONTWAIT);
    if( (ret != msg_len) && (_msg->len != msg_len) )
    	return -EAGAIN;
    *msg = _msg;
    return 0;
}


