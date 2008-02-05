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

typedef enum {NORMAL,SHELL} output_mode;
output_mode mode = NORMAL;

#define print_error(fmt,args...)							\
	if( mode == NORMAL ){ printf(fmt "\n", ## args);	}	\
	else { if( mode == SHELL ) printf("eoc_error=1\n"); }


typedef enum {NONE,CHANNEL,CONF_PROF} type_t;
typedef enum {NOACT,ADD,DEL,CHNG,DUMP} action_t;
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
		   "  -n  --annex={AnnexA | AnnexB}\tSetup annex\n"
		   "  -l, --lrate=<rate>\tSet line rate value\n"
		   "  -f, --pwr-feed={on|off}\tSet SHDSL channel power feed\n"
		   "  -u, --dump\tDump configuration to conf file (use without other options)\n"
		   "Output mode (default is user normal):\n"
		   "  -s, --row-shell\tShell capable output\n"
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
			print_error("Error: no --master option. Set default to \"slave\"\n");
			master = 0;
		}
		req = new app_frame(APP_ADD_CHAN,APP_SET,app_frame::REQUEST,1,name);
		chan_add_payload *p = (chan_add_payload *)req->payload_ptr();
		p->master = master;
		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			print_error("Bad message from eocd\n");
			goto exit;
		}
		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			print_error("Bad message from eocd\n");
			goto exit;
		} 
		if( ret = resp->is_negative() ){ // no such unit or no net_side
			print_error("Cannot add channel \"%s\": %s\n",name,err_strings[ret-1]);
			goto exit;
		}

		if( mode == NORMAL )
			printf("Channel \"%s\" added successfully\n",name);
		if( reg_num<0 && !cprof && active<0 )
			goto exit;
		delete req;
		req = NULL;
		delete resp;
		resp = NULL;
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
			print_error("Bad message from eocd\n");
			goto exit;
		}

		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			print_error("Bad message from eocd\n");
			goto exit;
		} 
		if( (ret = resp->is_negative() ) ){ 
			print_error("Cannot change channel \"%s\": (%d) %s\n",name,ret,err_strings[ret-1]);
			goto exit;
		}
		if( mode == NORMAL )
			printf("Channel \"%s\" changed successfully\n",name);
		break;
	}
	case DEL:{
		req = new app_frame(APP_DEL_CHAN,APP_SET,app_frame::REQUEST,1,name);
		chan_add_payload *p = (chan_add_payload *)req->payload_ptr();
		
		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			print_error("Bad message from eocd\n");
			goto exit;
		}
		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			print_error("Bad message from eocd\n");
			goto exit;
		} 
		if( ret = resp->is_negative() ){ // no such unit or no net_side
			print_error("Cannot delete channel \\\"%s\\\":(%d) %s\n",name,ret,err_strings[ret-1]);
			goto exit;
		}
		if( mode == NORMAL )
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
process_profile(char *name,action_t action,annex_t annex,power_t power,int lrate,int shell_out)
{
	app_frame *req=NULL, *resp=NULL;
 	int ret = 0;
 	char *buf;
	u8 add_and_config = 0;

	switch( action ){
	case ADD:{
		req = new app_frame(APP_ADD_CPROF,APP_SET,app_frame::REQUEST,1,"");
		cprof_add_payload *p = (cprof_add_payload*)req->payload_ptr();
		strncpy(p->pname,name,SNMP_ADMIN_LEN);
		//		printf("step0\n");
		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			print_error("Bad message from eocd\n");
			goto exit;
		}
		//		printf("step1\n");
		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			print_error("Bad message from eocd\n");
			goto exit;
		} 
		//		printf("step2\n");
		if( (ret=resp->is_negative()) ){ // no such unit or no net_side
			print_error("Cannot add configuration profile \"%s\": %s\n",name,err_strings[ret-1]);
			if( mode == SHELL )
				printf("err_string=\"Cannot add configuration profile \"%s\": %s\"\n",name,err_strings[ret-1]);				
			goto exit;
		}
		//		printf("step3\n");
		if( annex<0 && lrate<0 ){
			if( mode == NORMAL )
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
		req = new app_frame(APP_CPROF,APP_SET,app_frame::REQUEST,1,"");
		cprof_payload *p = (cprof_payload*)req->payload_ptr();
		span_conf_profile_t *mconf = &p->conf;
		cprof_changes *c = (cprof_changes *)req->changelist_ptr();
	
		memset(p,0,sizeof(*p));
		memset(c,0,sizeof(*c));
		strcpy(p->pname,name);
		if( lrate>=0 ){
			c->rate = 1;
			mconf->rate = lrate;
		}
		if( annex != err_annex ){
			c->annex = 1;
			mconf->annex = annex;
		}

		if( power != err_power ){
			c->power = 1;
			mconf->power = power;
		}
		
		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			print_error("Bad message from eocd\n");
			goto exit;
		}

		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			print_error("Bad message from eocd\n");
			goto exit;
		} 

		if( (ret=resp->is_negative()) ){
			print_error("Cannot change configuration profile \"%s\": %s\n",name,err_strings[ret-1]);
			if( mode == SHELL )
				printf("err_string=\"Cannot change configuration profile \"%s\": %s\"\n",name,err_strings[ret-1]);
			goto exit;
		}
		if( mode == NORMAL ){
			if( add_and_config )
				printf("Configuration profile \"%s\" added successfully\n",name);
			else
				printf("Configuration profile \"%s\" changed successfully\n",name);
		}
		break;
	}
	case DEL:{
		req = new app_frame(APP_DEL_CPROF,APP_SET,app_frame::REQUEST,1,"");
		cprof_del_payload *p = (cprof_del_payload*)req->payload_ptr();
		strcpy(p->pname,name);

		cli->send(req->frame_ptr(),req->frame_size());
		cli->wait();
		int size = cli->recv(buf);
		if( size<=0 ){
			print_error("Bad message from eocd\n");
			goto exit;
		}

		resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			print_error("Bad message from eocd\n");
			goto exit;
		} 

		if( (ret=resp->is_negative()) ){ // no such unit or no net_side
			print_error("Cannot delete configuration profile \"%s\":(%d) %s\n",name,ret,err_strings[ret-1]);
			if( mode == SHELL )
				printf("err_string=\"Cannot delete configuration profile \\\"%s\\\":(%d) %s\"\n",name,ret,err_strings[ret-1]);
			goto exit;
		}
		if( mode == NORMAL )
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

int dump_configuration()
{
	app_frame *req=NULL, *resp=NULL;
 	int ret = 0;
 	char *buf;
	u8 add_and_config = 0;

	req = new app_frame(APP_DUMP_CFG,APP_SET,app_frame::REQUEST,1,"");
	cli->send(req->frame_ptr(),req->frame_size());
	cli->wait();
	int size = cli->recv(buf);
	if( size<=0 ){
		print_error("Bad message from eocd\n");
		return 0;
	}
	resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
		print_error("Bad message from eocd\n");
		return 0;
	} 
	if( (ret=resp->is_negative()) ){ // no such unit or no net_side
		print_error("Cannot dump configuration to disk: %s\n",err_strings[ret-1]);
		if( mode == SHELL )
			printf("err_string=\"Cannot dump configuration to disk: %s\"\n",err_strings[ret-1]);				
		return 0;
	}
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
	annex_t annex = err_annex;
	power_t power = err_power;
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
			{"pwr-feed",1,0,'f'},
			{"lrate",1,0,'l'},
			{"row-shell",0,0, 's'},
			{"dump",0,0, 'u'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0}
		};

		int c = getopt_long (argc, argv, "o:a:d:c:m:r:p:v:f:n:l:shu",
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
			print_error("error --object argument: %s\n",optarg);
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
				print_error("error --master argument may be 0 or 1\n");
				return 0;
			}
			break;
		case 'r':
			reg_num = strtol(optarg,&endp,10);
			if( endp == optarg ){
				print_error("error --reg-num argument\n");
				return 0;
			}
			break;
		case 'p':
			cprof = strdup(optarg);
			break;
		case 'v':
			active = strtol(optarg,&endp,10);
			if( endp == optarg || (active!=0 && active!=1) ){
				print_error("error --active argument\n");
				return 0;
			}
			break;
		case 'n':
			annex = string2annex(optarg);
			if( annex<0 ){
				print_error("error --annex argument\n");
				return 0;
			}
			break;
		case 'f':
			power = string2power(optarg);
			if( power < 0 ){
				print_error("error --pwr-feed argument\n");
				return 0;
			}
			break;
		case 'l':
			lrate = strtol(optarg,&endp,10);
			if( endp == optarg || lrate<-1 ){
				print_error("error --lrate argument\n");
				return 0;
			}
			break;
		case 's':
			mode = SHELL;
			break;
		case 'u':
			action = DUMP;
			break;
		}
	}

	if( mode == NORMAL ){
		printf("----------------- Summary cmdline options -------------------\n");
		printf("Result:\n"
			   "type=%d,action=%d,name=%s,master=%d\n"
			   "cprof=%s,reg-num=%d,active=%d,annex=%d\n"
			   "lrate=%d,sout=%d,power=%d\n",
			   type,action,name,master,cprof,reg_num,
			   active,annex,lrate,shell_out,power);
		printf("----------------- Summary cmdline options -------------------\n");
	}

	if( type == NONE && action != DUMP ){
		print_usage(argv[0]);
		return 0;
	}

    cli = new app_comm_cli(sock_name);
    if( !cli->init_ok() ){
		print_error("Cannot connect to %s\n",sock_name);
		return 0;
    }

	if( type == NONE && action == DUMP ){
		dump_configuration();
		return 0;
	}


	if( !name ){
		print_error("Error: action option or option argument is missing\n");
		return 0;
	}

	switch( type ){
	case CHANNEL:
		if( (annex!=-1 || lrate!=-1) && mode == NORMAL ){
			printf("Warning: --annex & --lrate is ignored in channel mode\n");
		}
		process_channel(name,action,master,reg_num,cprof,active,shell_out);
		break;
	case CONF_PROF:
		if( (master!=-1 || cprof || reg_num!=-1 || active!=-1) && mode == NORMAL ){
			printf("Warning: --master, --reg_num, --active is ignored in channel mode\n");
		}
		process_profile(name,action,annex,power,lrate,shell_out);
		break;
	}

    return 0;
}

