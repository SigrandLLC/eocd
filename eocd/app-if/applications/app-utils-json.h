#ifndef APP_UTILS_JSON
#define APP_UTILS_JSON

#include "app-utils.h"

//--------- JASON mode output functions -------------//
int json_init();
int json_flush();
void do_indent(int indent);

//-------------- Error processing -------------------//
void json_error_fmt(int ret,int indent = 0);
void json_error_fmt(char *s,int indent = 0);
#define json_error(fmt,args...) { \
		char str[256]; \
		snprintf(str,256,"%s: "fmt,__FUNCTION__, ## args); \
		json_error_fmt(str); \
	}
#define json_errcode(ret) json_error_fmt(ret);

// -------------- Output info in JSON -----------------//
void json_sensor(int indent,int snum,int cur,int cnt);
int json_print_short(struct eoc_channel *channels, int cnum);
int json_channels_list(struct eoc_channel *channels,int cnum);
int json_spanconf(int indent,channel_info_t &info);
int json_spanstat(int indent,channel_info_t &info);
int json_channel(int indent,channel_info_t &info);
int json_m15ints(int indent,channel_info_t &info,unit u,side s,int loop,int inum = -1);
int json_d1ints(int indent,channel_info_t &info,unit u,side s,int loop,int inum = -1);
int json_loop(int indent,channel_info_t &info,unit u,side s,int loop);
int json_side(int indent,channel_info_t &info,span_params_payload *p,unit u,side s);
int json_unit(int indent,channel_info_t &info,unit u);
// Profile objects
int json_cprofiles(profiles_info_t &info);

#endif
