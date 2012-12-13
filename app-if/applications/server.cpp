#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>

#include <app-if/app_comm_srv.h>
#include <app-if/app_frame.h>

void
listen(app_comm_srv &app_srv)
{
    char *buff;
    int size,conn;
    int ret;

    if( !app_srv.wait() )
	return;

//    while( (size = app_srv.recv(conn,buff) ) ){
    while( 1 ){
	size = app_srv.recv(conn,buff);
	if( !size )
	    return;
	app_frame fr(buff,size);
	if( !fr.frame_ptr() ){
	    delete buff;
	    continue;
	}
	fr.response();
	app_srv.send(conn,fr.frame_ptr(),fr.frame_size());
    }
}


main()
{
    app_comm_srv srv("./","sock");
    char *buff;
    int size,conn;

    while(1){
	listen(srv);
    }

    return 0;
}



/*	    if( !(srv.wait()) )
		continue;
	    while( (size = srv.recv(conn,buff) ) ){
		app_frame fr(buff,size);
		if( fr.frame_ptr() ){
		    printf("FRAME ID(%d), TYPE(%d), ROLE(%d)\n",fr.id(),fr.type(),fr.role());
		}
	    }
*/

