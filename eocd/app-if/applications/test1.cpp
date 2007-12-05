extern "C"{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
}

#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>

#include "app-utils.h"

int
main(int argc, char *argv[] )
{
    char iface[256];
    char *sock_name = "/var/eocd/eocd-socket";
	//    char *sock_name = "/home/artpol/eocd-socket";
    typedef enum {NONE,SHORT,FULL,EXACT} type_t;
    type_t type = NONE;
    int unit = -1;
	char *buf;

    // Connect to eocd server
    app_comm_cli cli(sock_name);
    if( !cli.init_ok() ){
		printf("Cannot connect to %s\n",sock_name);
		return 0;
    }
 
	{
		printf("Change profile span#3-------------------------\n");
		app_frame *req = new app_frame(APP_CPROF,APP_SET,app_frame::REQUEST,1,"");
		cprof_payload *p = (cprof_payload*)req->payload_ptr();
		span_conf_profile_t *mconf = &p->conf;
		cprof_changes *c = (cprof_changes *)req->changelist_ptr();
	
		memset(p,0,sizeof(*p));
		memset(c,0,sizeof(*c));
		strcpy(p->pname,"span#3");
		mconf->rate = 2304;
		c->rate = 1;


		cli.send(req->frame_ptr(),req->frame_size());
		cli.wait();
		int size = cli.recv(buf);
		if( size<=0 ){
			printf("Size<0\n");
			return 0;
		}

		app_frame *resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			printf("Bad message from eocd\n");
			delete resp;
			delete req;
			return 0;
		} 
		if( resp->is_negative() ){ // no such unit or no net_side
			printf("Request is dropped\n");
			return 0;;
		}
		printf("Request_successfull\n");
	}
    return 0;
}

