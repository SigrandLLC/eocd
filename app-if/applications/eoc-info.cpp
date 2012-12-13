extern "C"{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
}

#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>
#include <app-if/err_strings.h>

#include "app-utils.h"
#include "app-utils-json.h"
#include "app-utils-table.h"
#include "app-utils-accum.h"

#ifndef EOC_VER
#define EOC_VER "0.0"
#endif


output_t output = NORMAL;
object_t obj = NONE;
bool full = false;
bool reset_rel = false;
char *ifname = NULL;
char *profname = NULL;
unit u = unknown;
side s = no_side;
table_type_t ttype = BASE;

void print_usage(char *name)
{
    printf("eoc-info versoin: %s\n",EOC_VER);
	printf("Usage: %s [-s|-f] [-i ] [-u [--row]] \n"
		   "Output mode (default:normal=human readable):\n"
		   "  -j, --json    Output for WEB interface in Java Script Object Notation format\n"
		   "Object (default:channel):\n"
		   "  If no additional options provided - short output about supported \n"
		   "  channels/profiles displayed\n"
		   "  -c, --channel  Get information about channels\n"
		   "  -p, --profile  Get information about profiles\n"
		   "Target specification(default:full)\n"
		   "  -f, --full  Get information about all served channels/profiles\n"
		   "  For channels:\n"
		   "    -i, --iface=<iface>    Get information about channel <iface>\n"
		   "    -u, --unit=<unit>      STU-C,STU-R,SRU1...SRU8 (use only with \'-i\')\n"
		   "    -e, --endpoint=<endp>  Set endpoint to <endp> (use only with \'-i\')\n"
		   "  For profiles:\n"
		   "    -n, --profname=<prof>  Get information about profile <prof>\n"
		   "Requested information for channel (default:base):\n"
		   "  -b, --base      Gate basic information: SNR Marg.,Loop atten.,ES,SES,LOSWS,UAS\n"
		   "  -r, --relative  Same as basic but relative counters also displayed\n"
		   "  -s, --sensors   Get information about sensors\n"
		   "  -v, --sens-full Full informations about sensors\n"
		   "  -m, --min15int  Get information about 15-munute intervals (conflict with -d)\n"
		   "  -d, --day1int   Get information about 1-day intervals (conflict with -m)\n"
           "Actions:\n"
		   "  -a, --relative-rst  Reset relative counters\n"
		   "Help page:\n"
		   "  -h, --help  This page\n"
		   ,name);
}

// --------------- CHANNEL objects processing -------------------------//

void
process_channel(app_comm_cli &cli)
{
	struct eoc_channel channels[MAX_CHANNELS];
	int chan_cnt;

	if( accum_channel_list(cli,channels,chan_cnt,output) ){
		exit(0);
	}
/*
printf("GETTED channel list: ");
for(int i=0; i< chan_cnt; i++){
	printf("%s ", channels[i].name);
}
printf("\n");
*/

	if( ifname ){ // Channel name was specified
		channel_info_t info;
		switch( output ){
		case NORMAL:
			info.tbl_type = ttype;
			break;
		case JSON:
			info.tbl_type = TBL_FULL;
			break;
		}

		if( !chan_cnt ){
			print_error(output,"No channels served");
			exit(0);
		}

		bool found = false;
		int index = -1;
		for(int i=0;i<chan_cnt;i++){
			if( !strcmp(ifname,channels[i].name) ){
				found = true;
				index = i;
				break;
			}
		}

		if( !found ){
			print_error(output,"No requested channel found: %s",ifname);
			exit(0);
		}

		// for JSON we always display info by units
		if( output == JSON ){
			init_chan_info(channels[index],info);
			if( accum_channel(cli,info,output,u) )
				exit(0);
			if( u == unknown ){
				// printf("json_channel(0,info);\n");
				json_channel(0,info);
			}else{
				// printf("json_unit(0,info,%s);\n",unit2string(u));
				json_unit(0,info,u);
			}
			json_flush();
			return;
		}

		if( u == unknown ){
			init_chan_info(channels[index],info);
			if( accum_channel(cli,info,output,BCAST) )
				exit(0);
			// printf("table_channels(&info,1);\n");
			table_channels(&info,1);
		}else{
			init_chan_info(channels[index],info);
			if(accum_channel(cli,info,output,u))
				exit(0);
			if(s==no_side){
				//printf("table_unit(&info,%s);\n", unit2string(u));
				table_unit(info, u);
			}else{
				//printf("table_side(&info,%s,%s);\n", unit2string(u),side2string(s));
				table_side(info, u, s);
				table_delim(info);
			}
		}
	}else{
		// Accumulate information about all channels
		if( !full ){
			// Print short information about sered channels
			switch( output ){
			case NORMAL:{
				channel_info_t *chan_infos = new channel_info_t[chan_cnt];
				for(int i=0;i<chan_cnt;i++){
					init_chan_info(channels[i],chan_infos[i]);
					chan_infos[i].tbl_type = SHORT;
					if( accum_channel(cli,chan_infos[i],output) )
						exit(0);
				}
				table_print_short(chan_infos,chan_cnt);
				delete[] chan_infos;
				break;
			}
			case JSON:
				json_print_short(channels,chan_cnt);
				json_flush();
				break;
			}
		}else{
			if( !chan_cnt ){
				print_error(output,"No channels served");
				exit(0);
			}

			if( output == JSON ){
				channel_info_t info;
				init_chan_info(channels[0],info);
				info.tbl_type = TBL_FULL;
				if( accum_channel(cli,info,output) )
					exit(0);
				json_channel(0,info);
				json_flush();
				return;
			}

			channel_info_t *chan_infos = new channel_info_t[chan_cnt];
			for(int i=0;i<chan_cnt;i++){
				init_chan_info(channels[i],chan_infos[i]);
				chan_infos[i].tbl_type = ttype;
				if( accum_channel(cli,chan_infos[i],output,BCAST) )
					exit(0);
			}
			table_channels(chan_infos,chan_cnt);
		}
	}
}

// --------------- PROFILE objects processing -------------------------//

void
process_profile(app_comm_cli &cli)
{
	profiles_info_t info;

	if( output == JSON ){
		accum_profiles(cli,info,output);
		json_cprofiles(info);
		json_flush();
		return;
	}

	if(profname){
		accum_profiles(cli,info,output,profname);
	}else{
		accum_profiles(cli,info,output);
	}
	table_cprofiles(info);
}

// --------------- MAIN -----------------------------------------------//
int
main(int argc, char *argv[] )
{
    char iface[256] = "";
    char *sock_name = "/var/eocd/eocd-socket";

    // process command line arguments here
    while (1) {
        int option_index = -1;
    	static struct option long_options[] = {
		{"json", 0, 0, 'j'},
		{"channel", 0, 0, 'c'},
		{"profile", 0, 0, 'p'},
		{"full", 0, 0, 'f'},
		{"interface", 1, 0, 'i'},
		{"unit", 1, 0, 'u'},
		{"endpoint", 1, 0, 'e'},
		{"profname", 1, 0, 'p'},
		{"base",0, 0, 'b'},
		{"relative",0, 0, 'r'},
		{"sensors", 0, 0, 's'},
		{"sens-full", 0, 0, 'v'},
		{"ints15min", 0, 0, 'm'},
		{"ints1day", 0, 0, 'd'},
		{"help", 0, 0, 'h'},
		{"relative-rst", 0, 0, 'a'},
		{0, 0, 0, 0}
		};

		int c = getopt_long (argc, argv, "jcpfi:u:e:n:brsmdhav",long_options, &option_index);
        if (c == -1)
    	    break;
		switch (c) {
        case 'c':
			// Check that JSON was not setted
			if( obj <= HELP ){
				obj = CHANNEL;
			}
            break;
        case 'j':
			// highest object priority
			output = JSON;
    	    break;
		case 'p':
			obj = PROFILE;
			break;
		case 'f':
			full = true;
			break;
		case 'i':
			ifname = strndup(optarg,256);
			break;
		case 'n':
			profname = strndup(optarg,256);
			break;
		case 'u':{
			u = string2unit(optarg);
			if( u == unknown ){
				print_error(output,"Error unit name: %s\n",optarg);
				exit(1);
			}
			break;
		}
		case 'e':{
			s = string2side(optarg);
			if( s == no_side ){
				print_error(output,"Error endpoint name: %s\n",optarg);
				exit(1);
			}
			break;
		}
		case 'b':
			ttype = BASE;
			break;
		case 'r':
			ttype = RELA;
			break;
		case 's':
			ttype = SENS;
			break;
		case 'v':
			ttype = SENS_FULL;
			break;
		case 'm':
			ttype = INT15;
			break;
		case 'd':
			ttype = INT1D;
			break;
		case 'a':
			reset_rel = true;
			break;
		case 'h':
			if( obj == NONE ){
				obj = HELP;
			}
			break;
		}
    }

	switch( obj ){
	case NONE:
		obj = CHANNEL;
		break;
	case HELP:
		print_usage(argv[0]);
		exit(0);
	default:
		break;
	}

    // Connect to eocd server
	app_comm_cli *cli1;
	switch( output ){
	case NORMAL:
		cli1 = new app_comm_cli(sock_name);
		break;
	case JSON:
		// Initialize JASON pipe
		if( json_init() )
			return 0;
		cli1 = new app_comm_cli(sock_name,1);
		break;
	}
    if( !cli1->init_ok() ){
		print_error(output,"Cannot connect to %s\n",sock_name);
		return 0;
    }
	app_comm_cli &cli = *cli1;

	// reset relative counters if requested
	if( reset_rel ){
		if( obj == CHANNEL && ifname && u != unknown && s != no_side ){
			if( rst_relative(cli,ifname,u,s,output) )
				exit(1);
		}else{
			print_error(output,"Cannot reset relative counters. Not all components provided.\nNeed interface name,unit,side");
			exit(1);
		}
	}

    // Do requested work
	switch( obj ){
	case PROFILE:
		process_profile(cli);
		break;
	case CHANNEL:
		process_channel(cli);
		break;
	}
    return 0;
}
