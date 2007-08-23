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


void print_usage(char *name)
{
    printf("Usage: %s [options]\n"
	    "Options:\n"
	    "  -s, --short\t\tShort info about served channels\n"
	    "  -f, --full\t\tFull info about served channels\n"
	    "  -i, --iface=<iface>\tFull info about channel <iface>\n"
	    "  -u, --unit=<unit>\t(1=STU-C,2=STU-R,3-SRU1...) Use only with -i\n"
	    "  -h, --help\t\tThis page\n",
	    name);
}

void
unit_full(app_comm_cli *cli,char *chan,int i)
{
    unit u = (unit)(i+1);
    printf("%s(%s) full info:\n",chan,unit2string(u));
    print_endp_cur(cli,chan,u,net_side,0);
    print_endp_cur(cli,chan,u,cust_side,0);
}

void print_short_chan(app_comm_cli *cli,char *chan)
{
    app_frame *req = new app_frame(APP_SPAN_PARAMS,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
    
    cli->send(req->frame_ptr(),req->frame_size());
    if( cli->wait() ){
//	printf("Error while waiting\n");
	cli->wait();
    }
    int size = cli->recv(buf);
    if( size <=0 ){
	delete req;
	return;
    }
    
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
        printf("Bad message from eocd\n");
	goto err_exit;
    } 
    
    if( resp->is_negative() ){ // no such unit or no net_side
	goto err_exit;
    }
    {
    span_params_payload *p = (span_params_payload *)resp->payload_ptr();
    int repeaters = (p->link_establ) ? p->units-2 : p->units-1;
    printf(" %s: %d repeaters, %s\n",chan,(repeaters<0) ? 0 : repeaters,
	    (p->link_establ) ? "online" : "offline");
    }
err_exit:
    delete resp;
    delete req;
    return;
}

void print_exact(app_comm_cli *cli,char *chan,int unit)
{
    app_frame *req = new app_frame(APP_SPAN_PARAMS,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
    
    cli->send(req->frame_ptr(),req->frame_size());
    cli->wait();
    int size = cli->recv(buf);
    if( !size ){
	delete req;
	return;
    }
    
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
        printf("Bad message from eocd\n");
	goto err_exit;
    } 
    
    if( resp->is_negative() ){ // no such unit or no net_side
	goto err_exit;
    }
{
    span_params_payload *p = (span_params_payload *)resp->payload_ptr();
    if( unit<0 ){
        for(int i=0;i<p->units;i++){
	    unit_full(cli,chan,i);
	}
    }else{
	unit_full(cli,chan,unit-1);
    }
}
err_exit:
    delete resp;
    delete req;
    return;

}


void print_short(app_comm_cli *cli)
{
    app_frame *req = new app_frame(APP_SPAN_NAME,APP_GET,app_frame::REQUEST,1,"");
    char *buf;
    int flag = 0;
    
    printf("Short information about served channels:\n");
    do{
        cli->send(req->frame_ptr(),req->frame_size());
	cli->wait();
	int size = cli->recv(buf);
	if( size<=0 )
	    break;

        app_frame *resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
	    printf("%s: Bad message from eocd\n",__FUNCTION__);
	    delete resp;
	    delete req;
	    return;
	} 
	if( resp->is_negative() ){ // no such unit or no net_side
	    delete resp;
	    break;
	}
	
        span_name_payload *p = (span_name_payload*)resp->payload_ptr();
	for(int i=0;i<p->filled;i++){
	    print_short_chan(cli,p->name[i]);
	    
	}	
	if( !p->filled )
	    break;
	flag = !p->last_msg;
	req->chan_name(p->name[p->filled-1]);
	delete resp;
    }while( flag );

    delete req;
}


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
	for(int i=0;i<p->filled;i++){
	    printf("-----------------------------------------------------\n");
	    print_exact(cli,p->name[i],-1);
	}	
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

    // process command line arguments here
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
    	    {"short", 0, 0, 's'},
    	    {"full", 0, 0, 'f'},
    	    {"iface", 1, 0, 'i'},
    	    {"unit", 1, 0, 'u'},
    	    {"help", 0, 0, 'h'},
    	    {0, 0, 0, 0}
	};

	int c = getopt_long (argc, argv, "sfhi:u:",
                long_options, &option_index);
        if (c == -1)
    	    break;
	switch (c) {
        case 's':
	    if( type == NONE )
		type = SHORT;
            break;
        case 'f':
	    type = FULL;
	    printf("Requested full info\n");
    	    break;
	case 'i':
	    if( type != FULL ){
		type = EXACT;
		strncpy(iface,optarg,255);
	    }
	    break;
	case 'u':
	{
	    char *endptr;
	    unit = strtoul(optarg,&endptr,0);
	    if( optarg == endptr ){
		printf("Error argument of --unit option\n");
		return 0;
	    }
	}
	case 'h':
	    break;
	}
    }
    if( type == NONE ){    
	print_usage(argv[0]);
	return 0;
    }
    
    // Connect to eocd server
    app_comm_cli cli(sock_name);
    if( !cli.init_ok() ){
	printf("Cannot connect to %s\n",sock_name);
	return 0;
    }
    
    // Do requested work
    switch( type ){
    case SHORT:
	print_short(&cli);
	break;
    case EXACT:
	print_exact(&cli,iface,unit);
	break;
    case FULL:
	print_full(&cli);
	break;
    }
    return 0;
}

