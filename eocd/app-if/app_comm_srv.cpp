#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <app-if/app_comm_srv.h>


app_comm_srv::
app_comm_srv(char *sock_path,char *sock_name) : app_comm(sock_path,sock_name)
{
    struct sockaddr_un saun;
    struct stat sbuf;
    int s;
    int ret=0,len;
    
    // Check path exist
    if( (ret = stat(sock_path,&sbuf)) ){
	if( errno != ENOENT ){
	    eocd_perror("Error getting info about %s",sock_path);
	    return;
	}
	if( mkdir(sock_path,(S_IRWXU | S_IRGRP | S_IXGRP)) ){
	    eocd_perror("Cannot create dir %s",sock_path);
	    return;
	}
	if( stat(sock_path,&sbuf) ){
	    eocd_perror("Error creating dir %s",sock_path);
	    return;
	}
    }  
    if( !S_ISDIR(sbuf.st_mode) ){
	eocd_log(ERROR,"Error: %s is not directory",sock_path);
	return;
    }

    // Create socket
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    	eocd_perror("Cannot create socket");
	return;    
    }
			    
    if( unlink(sname) ){
	if( errno != ENOENT ){
	    eocd_perror("Cannot unlink (%s)",sname);
	    return;
	}
    }
    
    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path,sname);
    len = sizeof(saun.sun_family) + strlen(saun.sun_path);
    if(bind(s, (const sockaddr*)&saun, len) < 0) {
    	eocd_perror("Cannot bind server with socket (%s)",sname);
	return;
    }
    // open socket for listening
    if(listen(s,MAX_CONNECTIONS) < 0) {
    	eocd_perror("Error trying listen socket (%s)",sname);
	return;
    }
    
    // setup non blocking
    if( set_nonblock(s) )
	return; /* ?? */

    // setup descriptor
    sfd = s;
    conn_num = 0;

    return;			
}


int app_comm_srv::
build_select_list()
{
    int i;
    if( sfd < 0 )
	return -1; 
    // blank fd set
    FD_ZERO(&socks);
    // fill fd set
    FD_SET(sfd,&socks);
    hisock = sfd;
    for(i=0; i < conn_num; i++) {
        FD_SET(conn_fd[i],&socks);
	if ( conn_fd[i] > hisock)
	    hisock = conn_fd[i];
    }
    return 0;
}

int app_comm_srv::
new_connection()
{
	int i,ret=0;
	int nsock; /* Socket file descriptor for incoming connections */

	nsock = accept(sfd, NULL, NULL);
	if( nsock < 0) {
		eocd_perror("Error: while accept incoming connection of (%s)",sname);
		return -errno;
	}
	if( (ret = set_nonblock(nsock)) ){
		close(nsock);
		return ret;
	}

	if( conn_num == MAX_CONNECTIONS ){
		close(nsock);
		eocd_log(WARNING,"Close new connection - no room left");
		return -ENOMEM;
	}
	// add to socket list
	conn_fd[conn_num++] = nsock;
	return 0;
}

int app_comm_srv::
complete_wait()
{
    int i,j;
    int num_act=0;
    char tmp;
	
    memset(conn_act,0,sizeof(conn_act));	
    for (i=0; i<conn_num; i++) {
	if (FD_ISSET(conn_fd[i],&socks)){
	    if( ::recv(conn_fd[i],&tmp,1,MSG_PEEK|MSG_DONTWAIT) < 1 ){
		eocd_log(WARNING,"Connection closed");
		close(conn_fd[i]);
		// shift descriptors
		for(j=i;j<conn_num;j++)
		    conn_fd[j]=conn_fd[j+1];
		conn_num--;
		i--;
	    }else{
		conn_act[i] = 1;
		num_act++;
	    }
	}
    }
    if (FD_ISSET(sfd,&socks))
	new_connection();
    return num_act;
}


int app_comm_srv::
next_fd(){
    int i;
    for(i=0;i<conn_num;i++){
	if( conn_act[i] ){
	    conn_act[i] = 0;
	    return i;
	}
    }
    return -1;
}

int app_comm_srv::
send(int c_num,char *buf,size_t size)
{
    if( !c_num ){
	return _send(sfd,buf,size);
    }else{
	if( c_num > conn_num )
	    return -1;
	return _send(conn_fd[c_num-1],buf,size);
    }
}

ssize_t app_comm_srv::
recv(int &c_idx,char *&buf)
{
    if( (c_idx = next_fd()) <0 ) 
	return 0;
    c_idx++;
    return _recv(conn_fd[c_idx-1],buf);
}
