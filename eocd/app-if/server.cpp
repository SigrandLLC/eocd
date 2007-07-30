#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>

#include <app_comm_srv.h>
#include <app_frame.h>

main()
{
    app_comm_srv srv("./","socket");
    char *buff;
    int size,conn;
    
    while(1){
	    if( !(srv.wait()) )
		continue;
	    while( (size = srv.recv(conn,buff) ) ){
		app_frame fr(buff,size);
		if( fr.frame_ptr() ){
		    printf("FRAME ID(%d), TYPE(%d), ROLE(%d)\n",fr.id(),fr.type(),fr.role());
		}
	    }
    }

    return 0;
}
