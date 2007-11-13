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
#include <app-if/err_strings.h>

#include "app-utils.h"


void print_usage(char *name)
{
    printf("Usage: %s [-s|-f] [-i ] [-u [--row]] \n"
		   "Options:\n"
		   "  -s, --short\t\tShort info about served channels\n"
		   "  -f, --full\t\tFull info about served channels\n"
		   "  -i, --iface=<iface>\tFull info about channel <iface>\n"
		   "  -u, --unit=<unit>\t(1=STU-C,2=STU-R,3-SRU1...) Use only with -i\n"
		   "  -w, --row\t\tRow output (for use in scripts)\n"
		   "  -h, --help\t\tThis page\n"
		   "Channel control options\n"
		   "      --add-chan=<name>\t\tAdd SHDSL channel <name> to channel list\n"
		   "                       \t\tChannel is created with default values\n"
		   "      --del-chan=<name>\t\tDelete SHDSL channel <name> from channel list\n"
		   "      --change-chan=<name>\t\tChange SHDSL channel <name> settings\n"
		   "      --master=[0|1]\t\tSet mode of channel. Use with (--add-chan \n"
		   "                    \t\tand --change-chan)\n"
		   "  -r, --reg-num=<#>\t\tSet number of installed regenerators \n"
		   "                   \t\t(use with --change-chan)\n"
		   "  -c, --conf-prof=<name>\t\tSet configuration profile for channel\n"
		   "                        \t\tuse with --change-chan\n"
		   "  -a, --active=[0|1]\t\t1 - eocd can change device settings, 0- can not\n"
		   "Configuration profiles control options\n"
		   "      --add-cprof=<name>\t\tAdd configuration profile <name>\n"
		   "                       \t\tProfile is created with default values\n"
		   "      --del-cprof=<name>\t\tDelete configuration profile <name>\n"
		   "      --change-cprof=<name>\t\tChange configuration profile settings\n"
		   "  -n  --annex=[0|1]\t\t0=Annex A, 1=Annex B (use with --change-prof)\n"
		   "  -l, --line-rate=<rate>\t\tSet line rate value\n"
		   ,name);
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

int add_channel(app_comm_cli *cli,char *name,u8 mode)
{
	app_frame *req=NULL,*resp=NULL;
 	int ret = 0;
 	char *buf;
	
	req = new app_frame(APP_ADD_CHAN,APP_SET,app_frame::REQUEST,1,name);
	chan_add_payload *p = (chan_add_payload *)req->payload_ptr();
	p->master = mode;

	cli->send(req->frame_ptr(),req->frame_size());
	cli->wait();
	int size = cli->recv(buf);
	if( size<=0 ){
		printf("Bad message from eocd\n");
		goto err_exit;
	}
	resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
		printf("Bad message from eocd\n");
		goto err_exit;
	} 
	if( ret = resp->is_negative() ){ // no such unit or no net_side
		printf("Cannot add channel \"%s\": %s\n",name,err_strings[ret-1]);
		goto err_exit;
	}
	printf("Channel \"%s\" added successfully\n");
 err_exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}

int del_channel(app_comm_cli *cli,char *name)
{
	app_frame *req = NULL, *resp=NULL;
 	int ret = 0;
 	char *buf;
	chan_add_payload *p = (chan_add_payload *)req->payload_ptr();
	req = new app_frame(APP_DEL_CHAN,APP_SET,app_frame::REQUEST,1,name);

	cli->send(req->frame_ptr(),req->frame_size());
	cli->wait();
	int size = cli->recv(buf);
	if( size<=0 ){
		printf("Bad message from eocd\n");
		goto err_exit;
	}
	resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
		printf("Bad message from eocd\n");
		goto err_exit;
	} 
	if( ret = resp->is_negative() ){ // no such unit or no net_side
		printf("Cannot add channel \"%s\": %s\n",name,err_strings[ret-1]);
		goto err_exit;
	}

	printf("Channel \"%s\" added successfully\n");
 err_exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}


int 
change_channel(app_comm_cli *cli,char *name,int mode,char *profile,int rnum,int active)
{
	app_frame *req = NULL, *resp = NULL;
 	int ret = 0;
 	char *buf;

	req = new app_frame(APP_CHNG_CHAN,APP_SET,app_frame::REQUEST,1,name);
	chan_chng_payload *p = (chan_chng_payload*)req->payload_ptr();
	memset(p,0,sizeof(*p));

	if( mode==0 || mode ==1){ 
		p->master = mode;
		p->master_ch = 1;
	}
	if( rnum >= 0 ){
		p->rep_num = rnum;
		p->rep_num_ch = 1;
	}
	if( active==0 || active == 1 ){
		p->apply_conf = active;
		p->apply_conf_ch = 1;
	}
	if( profile ){
		strcpy(p->cprof,profile);
		p->cprof_ch = 1;
	}

	cli->send(req->frame_ptr(),req->frame_size());
	cli->wait();
	int size = cli->recv(buf);
	if( size<=0 ){
		printf("Bad message from eocd\n");
		goto err_exit;
	}

	resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
		printf("Bad message from eocd\n");
		goto err_exit;
	} 
	if( (ret = resp->is_negative() ) ){ // no such unit or no net_side
		printf("Cannot change channel \"%s\": %s\n",name,err_strings[ret-1]);
		goto err_exit;
	}
	printf("Channel \"%s\" changed successfully\n",name);
 err_exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}

int
add_cprofile(app_comm_cli *cli,char *name)
{
	app_frame *req=NULL, *resp=NULL;
 	int ret = 0;
 	char *buf;
	
	req = new app_frame(APP_ADD_CPROF,APP_SET,app_frame::REQUEST,1,"");
	span_add_cprof_payload *p = (span_add_cprof_payload*)req->payload_ptr();
	strncpy(p->pname,name,SNMP_ADMIN_LEN);

	cli->send(req->frame_ptr(),req->frame_size());
	cli->wait();
	int size = cli->recv(buf);
	if( size<=0 ){
		printf("Bad message from eocd\n");
		goto err_exit;
	}

	resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
		printf("Bad message from eocd\n");
		goto err_exit;
	} 

	if( (ret=resp->is_negative()) ){ // no such unit or no net_side
		printf("Cannot add configuration profile \"%s\": %s\n",name,err_strings[ret-1]);
		goto err_exit;
	}
	printf("Configuration profile \"%s\" added successfully\n");
 err_exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}	

int
del_cprofile(app_comm_cli *cli,char *name)
{
	app_frame *req=NULL, *resp=NULL;
 	int ret = 0;
 	char *buf;

	req = new app_frame(APP_DEL_CPROF,APP_SET,app_frame::REQUEST,1,"");
	span_add_cprof_payload *p = (span_add_cprof_payload*)req->payload_ptr();
	strcpy(p->pname,name);
	cli->send(req->frame_ptr(),req->frame_size());
	cli->wait();
	int size = cli->recv(buf);
	if( size<=0 ){
		printf("Bad message from eocd\n");
		goto err_exit;
	}

	resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
		printf("Bad message from eocd\n");
		goto err_exit;
	} 

	if( (ret=resp->is_negative()) ){ // no such unit or no net_side
		printf("Cannot delete configuration profile \"%s\": %s\n",name,err_strings[ret-1]);
		goto err_exit;
	}
	printf("Configuration profile \"%s\" deleted successfully\n");
 err_exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}
/*
int
change_profile(app_comm_cli *cli,char *name)
{
	app_frame *req=NULL,*resp=NULL;
 	int ret = 0;
 	char *buf;

	req = new app_frame(APP_SPAN_CPROF,APP_SET,app_frame::REQUEST,1,"");
	span_conf_prof_payload *p = (span_conf_prof_payload*)req->payload_ptr();
	span_conf_profile_t *mconf = &p->conf;
	span_conf_prof_changes *c = (span_conf_prof_changes *)req->changelist_ptr();
	
	memset(p,0,sizeof(*p));
	memset(c,0,sizeof(*c));
	strcpy(p->pname,"span#4");
	mconf->max_rate = 2304;
	c->max_rate = 1;


	cli->send(req->frame_ptr(),req->frame_size());
	cli->wait();
	int size = cli->recv(buf);
	if( size<=0 ){
		printf("Bad message from eocd\n");
		goto err_exit;
	}

	app_frame *resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
		printf("Bad message from eocd\n");
		goto err_exit;
	} 

	if( (ret=resp->is_negative()) ){ // no such unit or no net_side
		printf("Cannot change configuration profile \"%s\": %s\n",name,err_strings[ret-1]);
		goto err_exit;
	}
	printf("Configuration profile \"%s\" changed successfully\n",name);
 err_exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}

*/
int
main(int argc, char *argv[] )
{
    char iface[256];
    char *sock_name = "/var/eocd/eocd-socket";
	//    char *sock_name = "/home/artpol/eocd-socket";
    typedef enum {NONE,SHORT,FULL,EXACT,SET} type_t;
    type_t type = NONE;
    int unit = -1;
	int row_mode = 0;
	// Set variables
	int reg_num = -1;
	char *conf_prof = NULL;
	

    // process command line arguments here
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
		{"short", 0, 0, 's'},
		{"full", 0, 0, 'f'},
		{"iface", 1, 0, 'i'},
		{"unit", 1, 0, 'u'},
		{"help", 0, 0, 'h'},
		{"row", 0, 0, 'w'},
		{"add-profile",0,0,'1'},
		{"del-profile",0,0,'2'},
		{"change-profile",0,0,'3'},
		{"add-channel",0,0,'4'},
		{"del-channel",0,0,'5'},
		{"change-channel",0,0,'6'},
		{"reg-num",0,0, 'r'},
		{"conf-prof",0,0,'c'},
		{0, 0, 0, 0}
	};

		int c = getopt_long (argc, argv, "sfhi:u:wr:c:1:2:34:5:6",
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
		case 'w':
			{
				row_mode = 1;
				break;
			}
		case 'r':
			{ // Regenerators theoretical number
				type = SET;
				char *endptr;
				reg_num = strtol(optarg,&endptr,0);
				if( optarg == endptr ){
					printf("Error argument of --unit option\n");
					return 0;
				}
			}
		case 'c':
			{ // Configuration profile name for channel
				type = SET;
				conf_prof = strdup(optarg);
				break;
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

