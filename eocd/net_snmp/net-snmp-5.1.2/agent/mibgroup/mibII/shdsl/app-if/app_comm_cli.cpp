#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <app-if/app_comm_cli.h>
#include <eoc_debug.h>

app_comm_cli::
app_comm_cli(char *sock_name):app_comm(sock_name)
{
    struct sockaddr_un saun;
    struct stat sbuf;    
    int s;
    int ret=0,len;

    // Check path exist
    if( (ret = stat(sname,&sbuf)) ){
        PERROR("Problem with socket (%s)",sname);
        error_init = 1;
        return;
    }  
    if( !S_ISSOCK(sbuf.st_mode) ){
	PDEBUG(DERR,"Not a socket (%s)",sname);
        error_init = 1;	
	return;
    }

    // Create socket
    if ( (s = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0) {
    	PERROR("Cannot create socket (%s)",sname);
        error_init = 1;
	return;    
    }

    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path,sname);
	
    len = sizeof(saun.sun_family) + strlen(saun.sun_path);
    if (connect(s,(struct sockaddr*)&saun, len) < 0) {
	PERROR("Cannot connect to (%s)",sname);
        error_init = 1;
	sfd = -1;
	return;
    }
    sfd = s;
}

app_comm_cli::
~app_comm_cli()
{
    close(sfd);
}

int app_comm_cli::
complete_wait()
{
    if( sfd < 0 ){
	PDEBUG(DERR,"Error wile initialisation\n");
	return -1;
    }

    if (FD_ISSET(sfd,&socks))
	return 1;
    return 0;
}

int app_comm_cli::
send(char *buf,size_t size)
{
    if( sfd < 0 ){
	PDEBUG(DERR,"Error wile initialisation\n");
	return -1;
    }
    return _send(sfd,buf,size);

}

ssize_t app_comm_cli::
recv(char *&buf)
{
    if( sfd < 0 ){
	PDEBUG(DERR,"Error wile initialisation\n");
	return -1;
    }

    return _recv(sfd,buf);
}
