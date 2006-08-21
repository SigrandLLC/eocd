extern "C"{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>
#include <string.h>
#include <getopt.h>
}

#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>

char *
unit2string(unit u)
{
    static char buf[64];
    switch(u){
    case stu_c:
	sprintf(buf,"STU-C");
	break;
    case stu_r:
	sprintf(buf,"STU-R");
	break;
    case sru1:
	sprintf(buf,"SRU1");
	break;
    case sru2:
	sprintf(buf,"SRU2");
	break;
    case sru3:
	sprintf(buf,"SRU3");
	break;
    case sru4:
	sprintf(buf,"SRU4");
	break;
    case sru5:
	sprintf(buf,"SRU5");
	break;
    case sru6:
	sprintf(buf,"SRU6");
	break;
    case sru7:
	sprintf(buf,"SRU7");
	break;
    case sru8:
	sprintf(buf,"SRU8");
	break;
    default:
	sprintf(buf,"unknown unit");
    }
    return buf;
}

char *
side2string(side s)
{
    static char buf[64];
    switch(s){
    case net_side:
	sprintf(buf,"NetSide");
	break;
    case cust_side:
	sprintf(buf,"CustSide");
	break;
    default:
	sprintf(buf,"unknown side");
	break;
    }
    return buf;
}

int
print_cur_payload(endp_cur_payload *p )
{
    printf("  %s:\n",side2string((side)p->side));
    printf("    SNR Margin:\t\t%d\n",p->cur_snr);
    printf("    LoopAttenuation:\t%d\n",p->cur_attn);
//-------------------------------------------------------------//	
    printf("    Counters till startup: ");	
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u)\n",
	p->total.es,p->total.ses,p->total.crc,p->total.losws,p->total.uas);
}

int 
print_int_payload(endp_int_payload *p,char *display)
{
    printf("%s interval #%d: ",display,p->int_num);	
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u)\n",
	p->cntrs.es,p->cntrs.ses,p->cntrs.crc,p->cntrs.losws,p->cntrs.uas);
    return 0;
}


int
print_endp_cur(app_comm_cli *cli,char *chan,unit u,side s,int loop)
{
    char *b;
    int size;
    // Endpoint current
    endp_cur_payload *p,*p1;
    app_frame *req, *resp;
    
    req = new app_frame(APP_ENDP_CUR,APP_GET,app_frame::REQUEST,1,chan);
    p = (endp_cur_payload*)req->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = loop;
    cli->send(req->frame_ptr(),req->frame_size());
    cli->wait();
    size = cli->recv(b);
    resp = new app_frame(b,size);
    if( !resp->frame_ptr() ){
	printf("error requesting\n");
	return -1;
    } 
    if( resp->is_negative() ){ // no such unit or no net_side
	delete resp;
    } else {
	p1 = (endp_cur_payload*)resp->payload_ptr();
	if( p1->unit != p->unit || p1->side != p->side || p1->loop != p->loop ){
	    printf("Error: get information about different unit\n");
	    delete resp;
	    return -1;
	}
	print_cur_payload(p1);
	delete resp;
    }
    delete req;
    return 0;
}


int
print_endp_15m(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum)
{
    char *b;
    int size;
    // Endpoint current
    endp_15min_payload *p,*p1;
    app_frame *fr, *fr1;
    
    fr = new app_frame(APP_ENDP_15MIN,APP_GET,app_frame::REQUEST,1,chan);
    p = (endp_15min_payload*)fr->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = loop;
    p->int_num = inum;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
	printf("error requesting\n");
	return -1;
    } 
    if( fr1->is_negative() ){ // no such unit or no net_side
	delete fr1;
	printf("Requested component: unit(%s),side(%s),loop(%d),int(%d) NOT FOUND\n",
		unit2string(u),side2string(s),loop,inum);
	delete fr;
	return -1;
    } else {
	p1 = (endp_15min_payload*)fr1->payload_ptr();
	if( p1->unit != p->unit || p1->side != p->side 
	    || p1->loop != p->loop || p1->int_num != p->int_num ){
	    printf("Error: get information about different unit\n");
	    delete fr1;
	    return -1;
	}
	print_int_payload(p1,"15min");
	delete fr1;
    }
    delete fr;
    return 0;
}


int
print_endp_1d(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum)
{
    char *b;
    int size;
    // Endpoint current
    endp_1day_payload *p,*p1;
    app_frame *fr, *fr1;
    
    fr = new app_frame(APP_ENDP_1DAY,APP_GET,app_frame::REQUEST,1,chan);
    p = (endp_1day_payload*)fr->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = loop;
    p->int_num = inum;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
	printf("error requesting\n");
	return -1;
    } 
    if( fr1->is_negative() ){ // no such unit or no net_side
	delete fr1;
	printf("Requested component: unit(%s),side(%s),loop(%d),int(%d) NOT FOUND\n",
		unit2string(u),side2string(s),loop,inum);
	delete fr;
	return -1;
    } else {
	p1 = (endp_1day_payload*)fr1->payload_ptr();
	if( p1->unit != p->unit || p1->side != p->side 
	    || p1->loop != p->loop || p1->int_num != p->int_num ){
	    printf("Error: get information about different unit\n");
	    delete fr1;
	    return -1;
	}
	print_int_payload(p1,"1day");
	delete fr1;
    }
    delete fr;
    return 0;
}
