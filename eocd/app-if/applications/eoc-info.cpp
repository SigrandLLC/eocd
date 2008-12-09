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

#ifndef EOC_VER
#define EOC_VER "0.0"
#endif


typedef enum {NORMAL,SHELL,JASON} output_mode;
output_mode mode = NORMAL;
typedef enum {NONE,PSHORT,PEXACT,PFULL,SHORT,FULL,EXACT,RESET,SENSORS,PBO} type_t;
type_t type = NONE;
unit _unit = unknown;
side _side = no_side;
int _loop = -1;
int _dint = 0, _mint = 0;
char *prof=NULL;

#define print_error(fmt,args...)							\
	if( mode == NORMAL ){ printf(fmt "\n", ## args);	}	\
	else { if( mode == SHELL ) printf("eoc_error=1\n"); }   \


void print_usage(char *name)
{
    printf("eoc-info versoin: %s\n",EOC_VER);
	printf("Usage: %s [-s|-f] [-i ] [-u [--row]] \n"
		   "Options:\n"
		   "  -s, --short\t\tShort info about served channels\n"
		   "  -f, --full\t\tFull info about served channels\n"
		   "  -i, --iface=<iface>\tGEt information about channel <iface>\n"
		   "  -u, --unit=<unit>\tSTU-C,STU-R,SRU1...SRU8, use only with \'-i\'\n"
		   "  -e, --endpoint=<endp>\tCustSide,NetSide, use only with \'-u\'\n"
		   "  -l, --loop=<loop#>\tLoop number:0,1,2,3; use only with  \'-e\'\n"
		   "  -m, --m15int=<#int>\t15 minutes interval number (conflicts with -d)\n"
		   "  -d, --d1int=<#int>\t1 day interval number (conflicts with -m)\n"
		   "  -t, --profile-list\tprofiles names list (only for row-shell & jason modes)\n"
		   "  -p, --profile[=<name>]\tInformation about profile <name>. If <name> is ommited\n"
		   "                        \tlists names of all profiles\n"
		   "  -a, --all-profiles\tInformation about all profiles in system\n"
		   "  -r, --row-shell\t\tRow shell output (for use in scripts)\n"
		   "  -n,--jason\t\tprint in JASON (Java Script Object Notation format)\n"
		   "  -v, --relative-rst\tReset relative counters\n"
		   "  -w, --show-slaves\tLists slave interfaces (use with -r & -s)\n"
		   "  -j, --sensors\tShows sensors state\n"
		   "  -o, --pbo\tPower backoff settings\n"
		   "  -h, --help\t\tThis page\n"
		   ,name);
}

void print_sensors(app_comm_cli &cli,char *chan,int indent=0)
{
    app_frame *req = new app_frame(APP_SENSORS,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
	
	((sensors_payload *)req->payload_ptr())->unit = _unit;
    cli.send(req->frame_ptr(),req->frame_size());
	cli.wait();
    int size = cli.recv(buf);
    if( size <=0 ){
		delete req;
		return;
    }
    
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
		print_error("error: bad message from eocd");
		goto err_exit;
    } 
    
    if( resp->is_negative() ){ // no such unit or no net_side
		print_error("error: negative response from server");
		goto err_exit;
    }
    {
		sensors_payload *p = (sensors_payload *)resp->payload_ptr();
		switch (mode){
		case NORMAL:
			printf(" %s %s: ",chan, unit2string(_unit));
			printf("\tSensor1: current=%d, count=%d\n",p->state.sensor1,p->sens1);
			printf("\tSensor2: current=%d, count=%d\n",p->state.sensor2,p->sens2);
			printf("\tSensor3: current=%d, count=%d\n",p->state.sensor3,p->sens3);
			break;
		case SHELL:
			printf("sens1_cur=%d\nsens1_cnt=%d\n",p->state.sensor1,p->sens1);
			printf("sens2_cur=%d\nsens2_cnt=%d\n",p->state.sensor2,p->sens2);
			printf("sens3_cur=%d\nsens3_cnt=%d\n",p->state.sensor3,p->sens3);
			break;
		case JASON:
			jason_sensor(indent,1,p->state.sensor1,p->sens1);
			jason_sensor(indent,2,p->state.sensor2,p->sens2);
			jason_sensor(indent,3,p->state.sensor3,p->sens3);
			break;
		}
    }
 err_exit:
    delete resp;
    delete req;
    return;
}


void print_pbo(app_comm_cli &cli,char *chan,int indent=0)
{
    app_frame *req = new app_frame(APP_PBO,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
    
	
    cli.send(req->frame_ptr(),req->frame_size());
	cli.wait();
    int size = cli.recv(buf);
    if( size <=0 ){
		delete req;
		return;
    }
    
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
		print_error("error: bad message from eocd");
		goto err_exit;
    } 
    
    if( resp->is_negative() ){ // no such unit or no net_side
		print_error("error: negative response from server");
		goto err_exit;
    }
    {
		chan_pbo_payload *p = (chan_pbo_payload *)resp->payload_ptr();
		switch( mode ){
		case NORMAL:
			printf("pbo_mode = %d, Value=%s\n",p->mode,p->val);
			break;
		case SHELL:
			printf("pbo_mode=%d\npbo_val=%s\n",p->mode,p->val);
			break;
		case JASON:
			jason_pbo(indent,p->mode,p->val);
			break;
		}
    }
 err_exit:
    delete resp;
    delete req;
    return;
}


void print_short_chan(app_comm_cli &cli,char *chan)
{
    app_frame *req = new app_frame(APP_SPAN_PARAMS,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
    
    cli.send(req->frame_ptr(),req->frame_size());
	cli.wait();
    int size = cli.recv(buf);
    if( size <=0 ){
		delete req;
		return;
    }
    
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
		print_error("error: bad message from eocd");
		goto err_exit;
    } 
    
    if( resp->is_negative() ){ // no such unit or no net_side
		print_error("error: negative response from server");
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

void 
print_exact(app_comm_cli &cli,char *chan)
{
    app_frame *req = new app_frame(APP_SPAN_PARAMS,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
    
    cli.send(req->frame_ptr(),req->frame_size());
    cli.wait();
    int size = cli.recv(buf);
    if( !size ){
		delete req;
		return;
    }
    
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
		print_error("error: bad message from eocd");
		goto exit;
    } 
    
    if( resp->is_negative() ){ // no such unit or no net_side
		print_error("error: negative response from server");
		goto exit;
    }
	{
		span_params_payload *p = (span_params_payload *)resp->payload_ptr();
		if( _unit == unknown ){
			// Print General statistics for all units,sides,loops
			for(int i=0;i<p->units;i++){
				for(int j=0;j<p->loops;j++){
					print_endp_cur(cli,chan,(unit)(i+1),net_side,j);
					print_endp_cur(cli,chan,(unit)(i+1),cust_side,j);
				}
			}
			goto exit;
		}else if( _unit > p->units ){
			print_error("error: no such unit: %s",unit2string(_unit));
			goto exit;
		}

		// Print general statistics for all sides,loops
		if( _side == no_side ){
			for(int j=0;j<p->loops;j++){
				print_endp_cur(cli,chan,_unit,net_side,j);
				print_endp_cur(cli,chan,_unit,cust_side,j);
			}
			goto exit;
		}

		// Print General statistics for all loops
		if( _loop<0 ){
			for(int j=0;j<p->loops;j++){
				print_endp_cur(cli,chan,_unit,_side,j);
			}
			goto exit;
		}

		// Print General statistics for fixed params
		if( !_mint && !_dint ){
			print_endp_cur(cli,chan,_unit,_side,_loop);
		}

		if( _mint ){
			print_endp_15m(cli,chan,_unit,_side,_loop,_mint);
		}else{
			print_endp_1d(cli,chan,_unit,_side,_loop,_dint);
		}
	}
 exit:
    delete resp;
    delete req;
    return;

}

void shell_exact(app_comm_cli &cli,char *chan)
{
    app_frame *req = new app_frame(APP_SPAN_PARAMS,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
	int ret = 0;

    cli.send(req->frame_ptr(),req->frame_size());
    cli.wait();
    int size = cli.recv(buf);
    if( !size ){
		delete req;
		return;
    }
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
		print_error("Bad message from server");
		goto exit;
    } 
    
    if( (ret=resp->is_negative()) ){ // no such unit or no net_side
		if( ret != ERNODB ){
			print_error();
			printf("err_srting=\"(%d) %s\"\n",ret,err_strings[ret-1]);
			goto exit;
		}else{
			shell_spanconf(cli,chan,0);
			goto exit;
		}
    }

	{
		span_params_payload *p = (span_params_payload *)resp->payload_ptr();
		if( _unit == unknown ){
			// Print General statistics for SRU-C, CustSide, loop0
			shell_channel(cli,chan,p);
			goto exit;
		}else if( _unit > p->units ){
			print_error();
			goto exit;
		}

		// Print general statistics for all NetSide,loop0
		if( _side == no_side ){
			if( _unit >= sru1 || _unit == stu_r ){
				if( shell_endp_cur(cli,chan,_unit,net_side,0) ){
					print_error("error: while printing NetSide current status");
				}
			}else if( shell_endp_cur(cli,chan,_unit,cust_side,0) ){
				print_error("error: while printing CustSide current status");
			}
			goto exit;
		}

		// Print General statistics for all loop0
		if( _loop<0 ){
			if( shell_endp_cur(cli,chan,_unit,_side,0) )
				print_error();
			goto exit;
		}

		// Print General statistics for fixed params
		if( !_mint && !_dint ){
			if( shell_endp_cur(cli,chan,_unit,_side,_loop) )
				print_error();
			goto exit;
		}

		if( _mint ){
			if( shell_endp_15m(cli,chan,_unit,_side,_loop,_mint) )
				print_error();
		}else{
			if( shell_endp_1d(cli,chan,_unit,_side,_loop,_dint) )
				print_error();
		}
	}
 exit:
    delete resp;
    delete req;
    return;
}


#define MAX_CHANS 256
int print_short(app_comm_cli &cli,int indent = 0)
{
    app_frame *req = new app_frame(APP_SPAN_NAME,APP_GET,app_frame::REQUEST,1,"");
    char *buf;
    int flag = 0;
	struct eoc_channel channels[MAX_CHANS];
	int ret = 0;
	
	int channels_num = 0;

	if( mode != SHELL )
		printf("Short information about served channels:\n");
    do{
        cli.send(req->frame_ptr(),req->frame_size());
		cli.wait();
		int size = cli.recv(buf);
		if( size<=0 )
			break;

        app_frame *resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			if( mode == JASON ){
				jason_error((char*)"error: bad message from eocd");
			}else{
				print_error("error: bad message from eocd");
			}
			delete resp;
			delete req;
			return -1;
		} 
		if( resp->is_negative() ){ // no such unit or no net_side
			if( mode == JASON ){
				jason_error((char*)"error: negative response from server");
			}else{
				print_error("error: negative response from server");
			}
			delete resp;
			return -1;
		}
	
        span_name_payload *p = (span_name_payload*)resp->payload_ptr();
		for(int i=0;i<p->filled && channels_num < MAX_CHANS;i++){
			switch( mode ){
			case NORMAL:
				if( p->spans[i].t == master )
					print_short_chan(cli,p->spans[i].name);
				break;
			case SHELL:
			case JASON:
				channels[channels_num].name = strdup(p->spans[i].name);
				channels[channels_num++].t = p->spans[i].t;
				break;
			}
		}	
		if( !p->filled )
			break;
		flag = !p->last_msg;
		req->chan_name(p->spans[p->filled-1].name);
		delete resp;
    }while( flag );
	
	switch ( mode ){
	case SHELL:
		printf("eoc_channels=\"");
		for(int i=0;i<channels_num;i++){
			printf("%s.%c ",channels[i].name,(channels[i].t==slave) ? 's' : 'm');
			free(channels[i].name);
		}
		printf("\"\n");
		break;
	case JASON:
		ret = jason_channels_list(channels,channels_num);
		break;
	}

    delete req;
	return ret;
}

void print_full(app_comm_cli &cli)
{
    app_frame *req = new app_frame(APP_SPAN_NAME,APP_GET,app_frame::REQUEST,1,"");
    char *buf;
    int flag = 0;
    
    printf("Full information about served channels:\n");
    do{
        cli.send(req->frame_ptr(),req->frame_size());
		cli.wait();
		int size = cli.recv(buf);
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
			_unit = unknown;
			if( p->spans[i].t == master ){
				printf("Channel: %s\n",p->spans[i].name);
				print_exact(cli,p->spans[i].name);
			}
		}	
		if( !p->filled )
			break;
		flag = !p->last_msg;
		req->chan_name(p->spans[p->filled-1].name);
		delete resp;
    }while( flag );
    delete req;
}

int rst_relative(app_comm_cli &cli,char *chan)
{
    app_frame *req = new app_frame(APP_LOOP_RCNTRST,APP_GET,app_frame::REQUEST,1,chan);
	char *buf;
    loop_rcntrst_payload *p;

	if( (_unit<0) || (_side<0) || (_loop<0) ){
		return -1;
	}

	p = (loop_rcntrst_payload *)req->payload_ptr();
	p->unit = (u8)_unit;
	p->side = (u8)_side;
	p->loop = (u8)_loop;
	cli.send(req->frame_ptr(),req->frame_size());
	cli.wait();
	int size = cli.recv(buf);
	if( size<=0 )
		return 0;

	app_frame *resp = new app_frame(buf,size);
	if( !resp->frame_ptr() ){
		print_error("Bad message from eocd\n");
		delete resp;
		delete req;
		return 0;
	} 
	int ret;
	if( (ret=resp->is_negative()) ){ // no such unit or no net_side
		print_error("Cannot reset relative counters: (%d) %s\n",ret,err_strings[ret-1]);
		delete resp;
		print_error("Cannot reset relative counters\n");
	}
	return 0;
}

int
main(int argc, char *argv[] )
{
    char iface[256] = "";
    char *sock_name = "/var/eocd/eocd-socket";
	
    // process command line arguments here
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
		{"short", 0, 0, 's'},
		{"full", 0, 0, 'f'},
		{"iface", 1, 0, 'i'},
		{"unit", 1, 0, 'u'},
		{"endpoint", 1, 0, 'e'},
		{"loop", 1, 0, 'l'},
		{"m15int", 1, 0, 'm'},
		{"d1int", 1, 0, 'd'},
		{"relative-rst", 0, 0, 'v'},
		{"profile-list",0, 0, 't'},
		{"all-profiles",0, 0, 'a'},
		{"profile", 1, 0, 'p'},
		{"row-shell", 0, 0, 'r'},
		{"jason", 0, 0, 'n'},
		{"sensors", 0, 0, 'j'},
		{"pbo", 0, 0, 'o'},
		{"help", 0, 0, 'h'},
		{0, 0, 0, 0}
	};

		int c = getopt_long (argc, argv, "sfhi:u:e:l:m:d:rnp:tavo",long_options, &option_index);
        if (c == -1)
    	    break;
		switch (c) {
        case 's':
			if( type == NONE )
				type = SHORT;
            break;
        case 'f':
			type = FULL;
    	    break;
		case 'i':
			if( type != FULL ){
				type = EXACT;
				strncpy(iface,optarg,255);
			}
			break;
		case 'u':
			{
				_unit = string2unit(optarg);
				if( _unit == unknown ){
					print_error("Error unit name: %s\n",optarg);
					return 0;
				}
			}
			break;
		case 'e':
			{
				_side = string2side(optarg);
				if( _side == no_side ){
					print_error("Error endpoint name: %s\n",optarg);
					exit(1);
				}
			}
			break;
		case 'l':
			{
				char *endp;
				_loop = strtoul(optarg,&endp,0);
				if( optarg == endp ){
					print_error("Wrong loop number: %s\n",optarg);
					exit(1);
				}
			}
			break;
		case 'm':
			{
				char *endp;
				_mint = strtoul(optarg,&endp,0);
				if( optarg == endp ){
					printf("Wrong 15 minutes interval number: %s\n",optarg);
					exit(1);
				}
				if( _dint > 0 ){
					printf("Conflicting options --m15int (-m) and --d1int (-d)\n");
					exit(1);
				}
			}
			break;
		case 'd':
			{
				char *endp;
				_dint = strtoul(optarg,&endp,0);
				if( optarg == endp ){
					printf("Wrong 1 day interval number: %s\n",optarg);
					exit(1);
				}
				if( _mint > 0 ){
					printf("Conflicting options --m15int (-m) and --d1int (-d)\n");
					exit(1);
				}
			}
			break;
		case 't':
			{
				if( type < PEXACT )
					type = PSHORT;
			}
			break;
		case 'p':
			{
				if( type < PFULL ){
					type = PEXACT;
					prof=strndup(optarg,SNMP_ADMIN_LEN);
				}
			}
			break;
		case 'a':
			{
				if( type < SHORT ){
					type = PFULL;
				}
			}
			break;
		case 'r':
			if( mode == NORMAL ){ // Jason has higher priopity
				mode = SHELL;
			}
			break;
		case 'n':
			mode = JASON;
			break;
		case 'v':
			type = RESET;
			break;
		case 'j':
			type = SENSORS;
			break;
		case 'o':
			type = PBO;
			break;
		case 'h':
			print_usage(argv[0]);
			return 0;
		}
    }
    
    // Connect to eocd server
	app_comm_cli *cli1;
	switch( mode ){
	case NORMAL:
		cli1 = new app_comm_cli(sock_name);
		break;
	case JASON:
		// Initialize JASON pipe
		if( jason_init() )
			return 0;
	case SHELL:
		// Both for SHELL & JASON
		cli1 = new app_comm_cli(sock_name,1);
		break;
	}
    if( !cli1->init_ok() ){
		print_error("Cannot connect to %s\n",sock_name);
		return 0;
    }
	app_comm_cli &cli = *cli1;
    
    // Do requested work
    switch(type){
	case PBO:
		if( !strlen(iface) ){
			print_error("No interface name given\n");
			return 0;
		}
		print_pbo(cli,iface);
		return 0;
	case SENSORS:
		if( _unit == unknown || !strlen(iface) ){
			print_error("No unit setted\n");
			return 0;
		}
		print_sensors(cli,iface);
		return 0;
	case RESET:
		rst_relative(cli,iface);
		break;
	case PSHORT:
		switch( mode ){
		case SHELL:
			shell_cprof_list(cli);
			break;
		case JASON:
			if( jason_cprof_full(cli) )
				return 0;
		default:
			break;
		}
		break;
	case PEXACT:
		switch( mode ){
		case SHELL:
			if( shell_cprof_info(cli,prof,0) )
				print_error();
			break;
		case JASON:
			if( jason_cprof_full(cli) )
				return 0;
		default:
			break;
		}
		break;
	case PFULL:
		switch( mode ){
		case SHELL:
			if( shell_cprof_full(cli) )
				print_error();
			break;
		case JASON:
			if( jason_cprof_full(cli) )
				return 0;
		default:
			break;
		}
		break;
    case FULL:
		if( mode != NORMAL ){
			if( print_short(cli) ) 
				return 0;
		}else
			print_full(cli);
		break;
    case SHORT:
		if( print_short(cli) )
			return 0;
		break;
    case EXACT:
		//		printf("D: Exact\n");
		switch(mode){
		case NORMAL:
			//			printf("D: NORMAL\n");
			print_exact(cli,iface);
			break;
		case SHELL:
			//			printf("D: SHELL\n");
			shell_exact(cli,iface);
			break;
		case JASON:
			//			printf("D: SHELL\n");
			if( jason_exact(0,cli,iface,_unit) )
				return 0;
			break;
		}
    }
	
	if( mode == JASON ){
		jason_flush();
	}
    return 0;
}

