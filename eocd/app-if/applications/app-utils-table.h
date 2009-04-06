#ifndef APP_UTILS_TABLE
#define APP_UTILS_TABLE

#include <app-if/err_strings.h>

#define CHAN_WIDTH 7
#define UNIT_WIDTH 6
#define SIDE_WIDTH 9
#define SNR_WIDTH 4
#define ATTN_WIDTH 4

#define RELA_WIDTH 2

#define SENSVAL_WIDTH 4
#define SENSTAT_WIDTH 2

#define DATE_WIDTH 9
#define TIME_WIDTH 6
#define MONI_WIDTH 3

#define TABLE_WIDTH (CHAN_WIDTH + UNIT_WIDTH + SIDE_WIDTH + SNR_WIDTH + ATTN_WIDTH)

#define REP_WIDTH 4
#define LINK_WIDTH 5
#define MODE_WIDTH 5
#define RATE_WIDTH 6
#define ANX_WIDTH 4
#define TCPAM_WIDTH 9
#define PWR_WIDTH 4 
#define COMPAT_WIDTH 9

#define table_error(fmt,args...) { \
		char str[256]; \
		snprintf(str,256,"%s: "fmt,__FUNCTION__, ## args); \
		table_error_fmt(str); \
	}

#define table_errcode(ret,chan) { \
	char str[256]; \
	if( strcmp(chan," ") ) \
		snprintf(str,256,"%s: Error(%d): (%s) %s\n", \
			__FUNCTION__,ret,chan,err_strings[ret - 1]); \
	else \
		snprintf(str,256,"%s: Error(%d): %s\n", \
			__FUNCTION__,ret,err_strings[ret - 1]); \
	table_error_fmt(str); \
}
	
void table_error_fmt(char *str);
//void table_error_fmt(int ret,char *chan);

void table_side(channel_info_t &info,unit u,side s);
void table_unit(channel_info_t &info,unit u);
void table_channels(channel_info_t info[],int cnt);
void table_print_short(channel_info_t info[],int cnt);
void table_delim(channel_info_t &info);
void table_delim(profiles_info_t &info);
int table_cprofiles(profiles_info_t &info);

#endif
