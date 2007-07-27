#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <app_comm_cli.h>

app_comm_cli::
app_comm_cli(char *sock_name):app_comm(sock_name)
{
    struct sockaddr_un saun;
    struct stat sbuf;    
    int s;
    int ret=0,len;

    // Check path exist
    if( (ret = stat(sname,&sbuf)) ){
	if( errno != ENOENT ){
	    eocd_perror("Error getting info about (%s)",sname);
	    return;
	}
    }  
    if( !S_ISSOCK(sbuf.st_mode) ){
	eocd_log(ERROR,"Not a socket (%s)",sname);
	return;
    }

    // Create socket
    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    	eocd_perror("Cannot create socket (%s)",sname);
	return;    
    }

    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path,sname);
	
    len = sizeof(saun.sun_family) + strlen(saun.sun_path);
    if (connect(s,(struct sockaddr*)&saun, len) < 0) {
	eocd_perror("Cannot connect to (%s)",sname);
	return;
    }
    sfd = s;
}


int app_comm_cli::
complete_wait()
{
    if (FD_ISSET(sfd,&socks))
	return 1;
    return 0;
}

int app_comm_cli::
send(int conn_num,char *buf,size_t size)
{
    return _send(sfd,buf,size);

}

ssize_t app_comm_cli::
recv(int &conn_num,char *&buf)
{
    return _recv(sfd,buf);
}
