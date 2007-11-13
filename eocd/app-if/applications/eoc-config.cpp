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


typedef enum {NONE,CHANNEL,CONF_PROF} type_t;
typedef enum {NOACT,ADD,DEL,CHNG} action_t;
app_comm_cli *cli;

void print_usage(char *name)
{
    printf("Usage: %s -o <type> [-a|-d|-c] <name> [Options] \n"
		   "Modes:\n"
		   "  -o, --object=<type>\tSelect type of object:\n"
		   "                     \tValid types: channel,conf-prof\n"
		   "Actions:\n"
		   "  -a, --add=<name>\tAdd new managed object <name> to EOC\n"
		   "  -d, --del=<name>\tDelete managed object <name> from EOC\n"
		   "  -c, --change=<name>\tChange managed object <name> in EOC\n"
		   "Options:\n"
		   "  Channel objects options\n"
		   "  -m, --master=[0|1]\tSet mode of channel\n"
		   "  -r, --reg-num=<#>\tSet number of installed regenerators \n"
		   "  -p, --cprof=<name>\tSet configuration profile <name> for channel\n"
		   "  -v, --active=[0|1]\t1-eocd can change device settings,0-can not\n"
		   "  Configuration profiles objects options\n"
		   "  -n  --annex=[0|1]\t0=AnnexA, 1=AnnexB\n"
		   "  -l, --lrate=<rate>\tSet line rate value\n"
		   "Output mode (default is user normal):\n"
		   "  -s, --shell-out\tShell capable output\n"
		   ,name);
}

int
process_channel(char *name,int action,int master,int reg_num,char *cprof,int active,int shell_out)
{
	app_frame *req=NULL,*resp=NULL;
 	int ret = 0;
 	char *buf;
	
	switch( action ){
	case ADD:{
		if( master<0 ){
			printf("Error: no --master option. Set default to \"slave\"\n");
		}
		req = new app_frame(APP_ADD_CHAN,APP_SET,app_frame::REQUEST,1,name);
		chan_add_payload *p = (chan_add_payload *)req->payload_ptr();
		p->master = master;
		printf("step1\n");
		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			printf("Bad message from eocd\n");
			goto exit;
		}
		printf("step2\n");
		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			printf("Bad message from eocd\n");
			goto exit;
		} 
		printf("step3\n");
		if( ret = resp->is_negative() ){ // no such unit or no net_side
			printf("Cannot add channel \"%s\": %s\n",name,err_strings[ret-1]);
			goto exit;
		}
		printf("Ret=%d\n",ret);
		printf("Channel \"%s\" added successfully\n",name);
		if( reg_num<0 && !cprof && active<0 )
			goto exit;
		printf("Free resources\n");
		delete req;
		req = NULL;
		delete resp;
		resp = NULL;
		printf("Change chan settings\n");
	}
	case CHNG:{
		req = new app_frame(APP_CHNG_CHAN,APP_SET,app_frame::REQUEST,1,name);
		chan_chng_payload *p = (chan_chng_payload*)req->payload_ptr();
		memset(p,0,sizeof(*p));
		
		if( master == 0 || master==1 ){
			p->master = master;
			p->master_ch = 1;
		}
		if( reg_num >= 0 ){
			p->rep_num = reg_num;
			p->rep_num_ch = 1;
		}
		if( active==0 || active == 1 ){
			p->apply_conf = active;
			p->apply_conf_ch = 1;
		}
		if( cprof ){
			strcpy(p->cprof,cprof);
			p->cprof_ch = 1;
		}

		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			printf("Bad message from eocd\n");
			goto exit;
		}

		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			printf("Bad message from eocd\n");
			goto exit;
		} 
		if( (ret = resp->is_negative() ) ){ 
			printf("Cannot change channel \"%s\": (%d) %s\n",name,ret,err_strings[ret-1]);
			goto exit;
		}
		printf("Channel \"%s\" changed successfully\n",name);
		break;
	}
	case DEL:{
		printf("Channel deleting\n");
		req = new app_frame(APP_DEL_CHAN,APP_SET,app_frame::REQUEST,1,name);
		chan_add_payload *p = (chan_add_payload *)req->payload_ptr();
		
		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			printf("Bad message from eocd\n");
			goto exit;
		}
		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			printf("Bad message from eocd\n");
			goto exit;
		} 
		if( ret = resp->is_negative() ){ // no such unit or no net_side
			printf("Cannot delete channel \"%s\": %s\n",name,ret,err_strings[ret-1]);
			goto exit;
		}
		printf("Channel \"%s\" deleted successfull\n",name);
		break;
	}
	}

 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}

int
process_profile(char *name,action_t action,int annex,int lrate,int shell_out)
{
	app_frame *req=NULL, *resp=NULL;
 	int ret = 0;
 	char *buf;
	u8 add_and_config = 0;

	switch( action ){
	case ADD:{
		req = new app_frame(APP_ADD_CPROF,APP_SET,app_frame::REQUEST,1,"");
		span_add_cprof_payload *p = (span_add_cprof_payload*)req->payload_ptr();
		strncpy(p->pname,name,SNMP_ADMIN_LEN);
		//		printf("step0\n");
		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			printf("Bad message from eocd\n");
			goto exit;
		}
		//		printf("step1\n");
		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			printf("Bad message from eocd\n");
			goto exit;
		} 
		//		printf("step2\n");
		if( (ret=resp->is_negative()) ){ // no such unit or no net_side
			printf("Cannot add configuration profile \"%s\": %s\n",name,err_strings[ret-1]);
			goto exit;
		}
		//		printf("step3\n");
		if( annex<0 && lrate<0 ){
			printf("Configuration profile \"%s\" added successfully\n",name);
			break;
		}
		delete req;
		req = NULL;
		delete resp;
		resp = NULL;
		add_and_config = 1;
	}
	case CHNG:{
		req = new app_frame(APP_SPAN_CPROF,APP_SET,app_frame::REQUEST,1,"");
		span_conf_prof_payload *p = (span_conf_prof_payload*)req->payload_ptr();
		span_conf_profile_t *mconf = &p->conf;
		span_conf_prof_changes *c = (span_conf_prof_changes *)req->changelist_ptr();
	
		memset(p,0,sizeof(*p));
		memset(c,0,sizeof(*c));
		strcpy(p->pname,name);
		if( lrate>=0 ){
			printf("Set Lrate to %d\n",lrate);
			c->max_rate = 1;
			mconf->max_rate = lrate;
		}
		if( annex>=0 ){
			printf("Set annex to %d\n",lrate);
			c->annex = 1;
			mconf->annex = (annex_t)(annex+1);
		}
		
		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			printf("Bad message from eocd\n");
			goto exit;
		}

		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			printf("Bad message from eocd\n");
			goto exit;
		} 

		if( (ret=resp->is_negative()) ){
			printf("Cannot change configuration profile \"%s\": %s\n",name,err_strings[ret-1]);
			goto exit;
		}
		if( add_and_config )
			printf("Configuration profile \"%s\" added successfully\n",name);
		else
			printf("Configuration profile \"%s\" changed successfully\n",name);
		break;
	}
	case DEL:{
		req = new app_frame(APP_DEL_CPROF,APP_SET,app_frame::REQUEST,1,"");
		span_add_cprof_payload *p = (span_add_cprof_payload*)req->payload_ptr();
		strcpy(p->pname,name);

		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			printf("Bad message from eocd\n");
			goto exit;
		}

		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			printf("Bad message from eocd\n");
			goto exit;
		} 

		if( (ret=resp->is_negative()) ){ // no such unit or no net_side
			printf("Cannot delete configuration profile \"%s\": %s\n",name,err_strings[ret-1]);
			goto exit;
		}
		printf("Configuration profile \"%s\" deleted successfully\n",name);
	}
	}

 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}	

int
main(int argc, char *argv[] )
{
    char *sock_name = "/var/eocd/eocd-socket";
    type_t type = NONE;
	action_t action = NOACT;
    char *name = NULL;
	char master = -1;
	char *cprof = NULL;
	char reg_num = -1;
	char active = -1;
	char annex = -1;
	int lrate = -1;
	char shell_out = -1;
	char *endp;

    // process command line arguments here

    while (1) {
        int option_index = -1;
    	static struct option long_options[] = 
			{
			{"object", 1, 0, 'o'},
			{"add", 1, 0, 'a'},
			{"del", 1, 0, 'd'},
			{"change", 1, 0, 'c'},
			{"master", 1, 0, 'm'},
			{"reg-num",1,0,'r'},
			{"cprof",1,0,'p'},
			{"active",1,0,'v'},
			{"annex",1,0,'n'},
			{"lrate",1,0,'l'},
			{"shell-out",0,0, 's'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0}
		};

		int c = getopt_long (argc, argv, "o:a:d:c:m:r:p:v:n:l:sh",
							 long_options, &option_index);
        if (c == -1)
    	    break;
		switch (c) {
        case 'o':
			if( !strncmp("channel",optarg,strlen("channel")) ){
				type = CHANNEL;
				break;
			}else if( !strncmp("conf-prof",optarg,strlen("conf-prof")) ){
				type = CONF_PROF;
				break;
			}
			printf("error --object argument: %s\n",optarg);
			return 0;
		case 'a':
			action = ADD;
			name = strdup(optarg);
			break;
		case 'd':
			action = DEL;
			name = strdup(optarg);
			break;
		case 'c':
			action = CHNG;
			name = strdup(optarg);
			break;
		case 'm':
			master = strtol(optarg,&endp,10);
			if( endp == optarg || (master!=0 && master!=1) ){
				printf("error --master argument may be 0 or 1\n");
				return 0;
			}
			break;
		case 'r':
			reg_num = strtol(optarg,&endp,10);
			if( endp == optarg ){
				printf("error --reg-num argument\n");
				return 0;
			}
			break;
		case 'p':
			cprof = strdup(optarg);
			break;
		case 'v':
			active = strtol(optarg,&endp,10);
			if( endp == optarg || (active!=0 && active!=1) ){
				printf("error --active argument\n");
				return 0;
			}
			break;
		case 'n':
			annex = strtol(optarg,&endp,10);
			if( endp == optarg || (annex!=0 && annex!=1) ){
				printf("error --annex argument\n");
				return 0;
			}
			break;
		case 'l':
			printf("lrate = %s\n",optarg);
			lrate = strtol(optarg,&endp,10);
			if( endp == optarg || lrate<-1 ){
				printf("error --lrate argument\n");
				return 0;
			}
			break;
		case 's':
			shell_out = 1;
			break;
		}
	}

	if( type == NONE )	
		print_usage(argv[0]);

	printf("----------------- Summary cmdline options -------------------\n");
	printf("Result:\n"
		   "type=%d,action=%d,name=%s,master=%d\n"
		   "cprof=%s,reg-num=%d,active=%d,annex=%d\n"
		   "lrate=%d,sout=%d\n",
		   type,action,name,master,cprof,reg_num,
		   active,annex,lrate,shell_out);
	printf("----------------- Summary cmdline options -------------------\n");

    cli = new app_comm_cli(sock_name);
    if( !cli->init_ok() ){
		printf("Cannot connect to %s\n",sock_name);
		return 0;
    }

	if( !name ){
		printf("Error: action option or option argument is missing\n");
		return 0;
	}

	switch( type ){
	case CHANNEL:
		if( annex!=-1 || lrate!=-1 ){
			printf("Warning: --annex & --lrate is ignored in channel mode\n");
		}
		process_channel(name,action,master,reg_num,cprof,active,shell_out);
		break;
	case CONF_PROF:
		if( master!=-1 || cprof || reg_num!=-1 || active!=-1 ){
			printf("Warning: --master, --reg_num, --active is ignored in channel mode\n");
		}
		process_profile(name,action,annex,lrate,shell_out);
		break;
	}

    return 0;
}

