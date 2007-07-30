#include <app-interface/app_comm.h>

int app_comm::
set_nonblock(int sock)
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

void app_comm::
build_select_list()
{
    // blank fd set
    FD_ZERO(&socks);
    // fill fd set
    FD_SET(sfd,&socks);
    hisock = sfd;
}

int app_comm::
wait()
{
    int count=0;
    struct timeval timeout;  /* Timeout for select */	
	
    while(1){
        timeout.tv_sec = 1;
	timeout.tv_usec = 0;
    	build_select_list();

	count = select(hisock+1,&socks,(fd_set *)0,(fd_set *)0, &timeout);
	if( count < 0) {
	    eocd_perror("Select");
	    return -errno;
	}
	if( count ){
		return complete_wait();
	}
    }
}

int app_comm::
_send(int fd,char *buf,size_t size)
{
    size_t nsize;
    char *nbuf = transp(buf,size,nsize);
    if( !nbuf )
	return -EAGAIN;
    if( ::send(fd,nbuf,nsize,0) != nsize )
        return -EAGAIN;
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
/*
    
    buf = untransp(frame,frame_len);
    if( !buf )
	return -ENOMEM;
*/    
    ret = ::recv(fd,(char*)frame,frame_len,MSG_DONTWAIT);
    if( frame_len != ret )
	return -EAGAIN;

    return frame_len;
}
