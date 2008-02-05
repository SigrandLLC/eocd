#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>

// Convertions between strings and vars 
char *unit2string(unit u);
char *side2string(side s);
char *annex2string(unit u);
char *power2string(side s);
unit string2unit(char *s);
side string2side(char *s);
annex_t string2annex(char *s);
power_t string2power(char *s);

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
