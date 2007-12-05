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

void print_full(app_comm_cli *cli)
{
    app_frame *req = new app_frame(APP_SPAN_NAME,APP_GET,app_frame::REQUEST,1,"");
    char *buf;
    int flag = 0;
    
    printf("Full information about served channels:\n");
    do{
        cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 )
			break;

        app_frame *resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			printf("Bad message from eocd\n");
			delete resp;
			delete req;
			return;
		} 
		if( resp->is_negative() ){ // no such unit or no net_side
			delete resp;
			break;
		}


		span_name_payload *p = (span_name_payload*)resp->payload_ptr();
		printf("Ifs: ");
		for(int i=0;i<p->filled;i++){
			printf("%s ",p->name[i]);
			//			printf("-----------------------------------------------------\n");
			//			print_exact(cli,p->name[i],-1);
		}	
		printf("\n");

		if( !p->filled )
			break;
		flag = !p->last_msg;
		req->chan_name(p->name[p->filled-1]);
		delete resp;
    }while( flag );

    delete req;

}




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
		printf("ADD channel dsl3-------------------------\n");
		app_frame *req = new app_frame(APP_ADD_CHAN,APP_SET,app_frame::REQUEST,1,"dsl3");
		chan_add_payload *p = (chan_add_payload *)req->payload_ptr();
		p->master = 0;
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

		printf("Change channel dsl3-------------------------\n");
		req = new app_frame(APP_CHNG_CHAN,APP_SET,app_frame::REQUEST,1,"dsl3");
		chan_chng_payload *p1 = (chan_chng_payload*)req->payload_ptr();
		memset(p,0,sizeof(*p1));
		p1->master = 1;
		p1->master_ch = 1;
		p1->rep_num = 1;
		p1->rep_num_ch = 1;
		p1->apply_conf = 1;
		p1->apply_conf_ch = 1;
		strcpy(p1->cprof,"span#3");
		p1->cprof_ch = 1;

		cli.send(req->frame_ptr(),req->frame_size());
		cli.wait();
		size = cli.recv(buf);
		if( size<=0 ){
			printf("Size<0\n");
			return 0;
		}

		resp = new app_frame(buf,size);
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

		printf("ADD channel dsl2-------------------------\n");
		req = new app_frame(APP_CHNG_CHAN,APP_SET,app_frame::REQUEST,1,"dsl2");
		p1 = (chan_chng_payload*)req->payload_ptr();
		memset(p,0,sizeof(*p1));
		p1->master = 0;
		p1->master_ch = 1;

		cli.send(req->frame_ptr(),req->frame_size());
		cli.wait();
		size = cli.recv(buf);
		if( size<=0 ){
			printf("Size<0\n");
			return 0;
		}

		resp = new app_frame(buf,size);
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
	
	{
		printf("ADD profile span#4-------------------------\n");
		app_frame *req = new app_frame(APP_ADD_CPROF,APP_SET,app_frame::REQUEST,1,"");
		cprof_add_payload *p = (cprof_add_payload*)req->payload_ptr();
		strcpy(p->pname,"span#4");
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
	{
		printf("DEL profile span#1-------------------------\n");
		app_frame *req = new app_frame(APP_DEL_CPROF,APP_SET,app_frame::REQUEST,1,"");
		cprof_add_payload *p = (cprof_add_payload*)req->payload_ptr();
		strcpy(p->pname,"span#4");
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
	{
		printf("Change profile span#3-------------------------\n");
		app_frame *req = new app_frame(APP_CPROF,APP_SET,app_frame::REQUEST,1,"");
		cprof_payload *p = (cprof_payload*)req->payload_ptr();
		span_conf_profile_t *mconf = &p->conf;
		cprof_changes *c = (cprof_changes *)req->changelist_ptr();
	
		memset(p,0,sizeof(*p));
		memset(c,0,sizeof(*c));
		strcpy(p->pname,"span#4");
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

