extern "C"{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>
#include <string.h>
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
	    "  --short (-s)\t\tShort info about served channels\n"
	    "  --full (-f)\t\tFull info about served channels\n"
	    "  --iface (-i) <iface>\tFull info about channel <iface>\n",
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
    printf(" %s: %d repeaters\n",chan,p->units);
    }
err_exit:
    delete resp;
    delete req;
    return;
}

void print_exact(app_comm_cli *cli,char *chan)
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
    for(int i=0;i<p->units;i++){
	unit_full(cli,chan,i);
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
	if( !size )
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
	if( !size )
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
	    print_exact(cli,p->name[i]);
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
//    char *sock_name = "/var/eocd/socket";
    char *sock_name = "/home/artpol/eocd-socket";
    typedef enum {NONE,SHORT,FULL,EXACT} type_t;
    type_t type = NONE;

    app_comm_cli cli(sock_name);
    if( !cli.init_ok() ){
	printf("Cannot connect to %s\n",sock_name);
	return 0;
    }

    // process command line arguments here
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
    	    {"short", 0, 0, 's'},
    	    {"full", 0, 0, 'f'},
    	    {"iface", 1, 0, 'i'},
    	    {0, 0, 0, 0}
	};

	int c = getopt_long (argc, argv, "sfi:",
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
	}
    }
    
    switch( type ){
    case NONE:
	print_usage(argv[0]);
	break;
    case SHORT:
	print_short(&cli);
	break;
    case EXACT:
	print_exact(&cli,iface);
	break;
    case FULL:
	print_full(&cli);
	break;
    }
    return 0;
}

/*    
    app_frame *fr = new app_frame(app_frame::SPAN_STATUS,app_frame::GET,app_frame::REQUEST,"dsl0");
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    printf("Receiving 1-st frame\n");
    int size = cli.recv(b);
    printf("Received %d bytes\n",size);
    app_frame *fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
	printf("Error in frame\n");
	return -1;
    }
    span_status_payload *p = (span_status_payload *)fr1->payload_ptr();
    printf("Act reps = %d\n",p->nreps);

    fr = new app_frame(app_frame::SPAN_CONF,app_frame::GET,app_frame::REQUEST,"dsl0");
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    span_conf_payload *p1 = (span_conf_payload *)fr1->payload_ptr();    
    printf("Conf reps = %d\n",p1->nreps);

    printf("Request STU-C inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    inventory_payload *p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = stu_c;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();    
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

    printf("Request STU-R inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = stu_r;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();    
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

    printf("Request SRU1 inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = sru1;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();    
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

    printf("Request SRU2 inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = sru2;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();    
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

    // Endpoint current
    printf("Request STU-C current info\n");
    endp_cur_payload *p3;
    fr = new app_frame(app_frame::ENDP_CUR,app_frame::GET,app_frame::REQUEST,"dsl0");
    p3 = (endp_cur_payload*)fr->payload_ptr();
    p3->unit = sru2;
    p3->side = EOC_unit::net_side;
    p3->loop = 1;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
    }

    printf("Request STU-R current info\n");
    fr = new app_frame(app_frame::ENDP_CUR,app_frame::GET,app_frame::REQUEST,"dsl0");
    p3 = (endp_cur_payload*)fr->payload_ptr();
    p3->unit = sru2;
    p3->side = EOC_unit::net_side;
    p3->loop = 1;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
    }


    ENDP_15MIN

    printf("Request SRU2 inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = sru2;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();    
	printf("Manuf = %s\n",p2->inv.ven_id);
    }


    ENDP_1DAY
    printf("Request SRU2 inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = sru2;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();    
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

*/
