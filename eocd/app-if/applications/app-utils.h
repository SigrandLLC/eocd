#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>


char *unit2string(unit u);
char *side2string(side s);
int print_cur_payload(endp_cur_payload *p );
int print_int_payload(endp_int_payload *p,char *display);
int print_endp_cur(app_comm_cli *cli,char *chan,unit u,side s,int loop);
int print_endp_15m(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum);
int print_endp_1d(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum);
