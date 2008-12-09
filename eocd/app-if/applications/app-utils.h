#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>

struct eoc_channel{
	char *name;
	dev_type t;
};


// Convertions between strings and vars 
char *unit2string(unit u);
unit string2unit(char *u);
char *side2string(side s);
side string2side(char *s);
char *annex2string(annex_t a);
annex_t string2annex(char *a);
char *tcpam2string(tcpam_t code);
char *tcpam2STRING(tcpam_t code);
tcpam_t string2tcpam(char *str);
char *power2string(power_t a);
power_t string2power(char *a);


// Normal mode output functions
int print_cur_payload(endp_cur_payload *p );
int print_int_payload(endp_int_payload *p,char *display);
int print_endp_cur(app_comm_cli &cli,char *chan,unit u,side s,int loop);
int print_endp_15m(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum);
int print_endp_1d(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum);

//---------- Shell mode output functions -------------//
int shell_channel(app_comm_cli &cli,char *chan,span_params_payload *p);
int shell_spanconf(app_comm_cli &cli,char *chan,int type=1);
int shell_endp_cur(app_comm_cli &cli,char *chan,unit u,side s,int loop);
int shell_endp_15m(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum);
int shell_endp_1d(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum);
int shell_cprof_info(app_comm_cli &cli,char *cprof,int ind);
int shell_cprof_full(app_comm_cli &cli);
int shell_cprof_list(app_comm_cli &cli);

//--------- JASON mode output functions -------------//
int jason_init();
int jason_flush();
void do_indent(int indent);
void jason_error(int ret,int indent = 0);
void jason_error(char *s,int indent = 0);
void jason_sensor(int indent,int snum,int cur,int cnt);
void jason_pbo(int indent,int mode,char *val);
void jason_short_channel(int indent,struct eoc_channel *chan);
int jason_channels_list(struct eoc_channel *channels,int cnum);
int jason_spanconf(int indent,app_comm_cli &cli,char *chan);
int jason_spanstat(int indent,app_comm_cli &cli,char *chan);
int jason_channel(int indent,app_comm_cli &cli,char *chan,span_params_payload *p);
int jason_m15ints(int indent,app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum = -1);
int jason_d1ints(int indent,app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum = -1);
int jason_loop(int indent, app_comm_cli &cli,char *chan,unit u,side s,int loop);
int jason_side(int indent,app_comm_cli &cli,char *chan,span_params_payload *p,unit u,side s);
int jason_exact(int indent,app_comm_cli &cli,char *chan,unit u);
int jason_cprof_full(app_comm_cli &cli);


