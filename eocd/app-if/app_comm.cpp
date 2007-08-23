#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <app-if/app_comm.h>
#include <eoc_debug.h>

int app_comm::
set_nonblock(int sock)
{
    int opts;

    opts = fcntl(sock,F_GETFL);
    if (opts < 0) {
    	PERROR("fcntl(F_GETFL)");
    	return -errno;
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock,F_SETFL,opts) < 0) {
    	PERROR("fcntl(F_SETFL)");
    	return -errno;
    }
    return 0;
}

int app_comm::
build_select_list()
{
    if( sfd < 0 )
	return -1;
    // blank fd set
    FD_ZERO(&socks);
    // fill fd set
    FD_SET(sfd,&socks);
    hisock = sfd;
    return 0;
}

int app_comm::
wait(int sec)
{
    int count=0;
    struct timeval timeout;  /* Timeout for select */	

    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    if( build_select_list() )
	return -1;

    count = select(hisock+1,&socks,(fd_set *)0,(fd_set *)0, &timeout);
    if( count < 0) {
        PERROR("Select");
        return -errno;
    }
    if( count ){
    	return complete_wait();
    }
    return count;
}

int app_comm::
_send(int fd,char *buf,size_t size)
{
    size_t nsize;
    if( (nsize=::send(fd,buf,size,0)) != size ){
        PDEBUG(DERR,"error: %d",nsize); 
        return -EAGAIN;
    }

    return 0;
}	

ssize_t app_comm::
_recv(int fd,char *&buf)
{
    char *frame = new char[BLOCK_SIZE];
    int frame_len;
    int ret;
    
    
    if( (frame_len = ::recv(fd,frame,BLOCK_SIZE,MSG_PEEK|MSG_DONTWAIT) ) <= 0 )
	return -EAGAIN;
    ret = ::recv(fd,(char*)frame,frame_len,MSG_DONTWAIT);
    if( frame_len != ret )
	return -EAGAIN;
    buf = frame;
    return frame_len;
}
