#ifndef APPUTILSACCUM_H_
#define APPUTILSACCUM_H_

#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>
#include "app-utils.h"
#include "app-utils-json.h"
#include "app-utils-table.h"

#define print_error(mode,fmt,args...) \
	if( mode == NORMAL ){ \
		table_error(fmt, ## args); \
	}else if(mode == JSON ) { \
		json_error(fmt, ## args); \
	}

#define print_errcode(mode,ret,chan) \
	if( mode == NORMAL ){ table_errcode(ret,chan); } \
	else if(mode == JSON ) { json_errcode(ret); }

int rst_relative(app_comm_cli &cli,char *chan,unit u,side s,output_t omode);
int accum_channel(app_comm_cli &cli,channel_info_t &info,output_t omode,unit u = unknown);

#define MAX_CHANNELS 64
int accum_channel_list(app_comm_cli &cli,struct eoc_channel channels[MAX_CHANNELS],int &chan_cnt,output_t omode);
int accum_profiles(app_comm_cli &cli, profiles_info_t &info,output_t omode,char *pname = NULL);

#endif /* APPUTILSACCUM_H_ */
