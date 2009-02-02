extern "C"{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <strings.h>
#include <stdlib.h>
}

#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>
#include "app-utils.h"

// unit-string conversions
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

unit
string2unit(char *u)
{
    static char buf[64];
	if( !strncasecmp(u,"stu-c",5) )
        return stu_c;
	else if( !strncasecmp(u,"stu-r",5) )
		return stu_r;
	else if( !strncasecmp(u,"sru1",4) )
		return sru1;
	else if( !strncasecmp(u,"sru2",4) )
		return sru2;
	else if( !strncasecmp(u,"sru3",4) )
		return sru3;
	else if( !strncasecmp(u,"sru4",4) )
		return sru4;
	else if( !strncasecmp(u,"sru5",4) )
		return sru5;
	else if( !strncasecmp(u,"sru6",4) )
		return sru6;
	else if( !strncasecmp(u,"sru7",4) )
		return sru7;
	else if( !strncasecmp(u,"sru8",4) )
		return sru8;
    return unknown;
}

// Side-string conversions
char*
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

side
string2side(char *s)
{
	if( !strncasecmp(s,"netside",7) )
		return net_side;
	else if( !strncasecmp(s,"custside",8) )
		return cust_side;
	else
		return no_side;
}

char *
annex2string(annex_t a)
{
	static char buf[64];
	switch( a ){
	case annex_a:
		strcpy(buf,"A");
		break;
	case annex_b:
		strcpy(buf,"B");
		break;
	default:
		strcpy(buf,"unknown");
		break;
	}
	return buf;
}

annex_t
string2annex(char *a)
{
	if( !strncasecmp(a,"AnnexA",6) )
		return annex_a;
	else if( !strncasecmp(a,"AnnexB",6) )
		return annex_b;
	return err_annex;
}


char *
tcpam2string(tcpam_t code)
{
	static char buf[64];
	switch( code ){
	case tcpam4:
		strcpy(buf,"tcpam4");
		break;
	case tcpam8:
		strcpy(buf,"tcpam8");
		break;
	case tcpam16:
		strcpy(buf,"tcpam16");
		break;
	case tcpam32:
		strcpy(buf,"tcpam32");
		break;
	case tcpam64:
		strcpy(buf,"tcpam64");
		break;
	case tcpam128:
		strcpy(buf,"tcpam128");
		break;
	default:
		strcpy(buf,"unknown");
		break;
	}
	return buf;
}

char *
tcpam2STRING(tcpam_t code)
{
	static char buf[64];
	switch( code ){
	case tcpam4:
		strcpy(buf,"TCPAM4");
		break;
	case tcpam8:
		strcpy(buf,"TCPAM8");
		break;
	case tcpam16:
		strcpy(buf,"TCPAM16");
		break;
	case tcpam32:
		strcpy(buf,"TCPAM32");
		break;
	case tcpam64:
		strcpy(buf,"TCPAM64");
		break;
	case tcpam128:
		strcpy(buf,"TCPAM128");
		break;
	default:
		strcpy(buf,"unknown");
		break;
	}
	return buf;
}

tcpam_t
string2tcpam(char *str)
{
	char *eptr;
	unsigned long tcpam = strtoul(str,&eptr,10);

	if( eptr == str ){
		return err_tcpam;
	}

	switch(tcpam){
	case 4: return tcpam4;
	case 8: return tcpam8;
	case 16: return tcpam16;
	case 32: return tcpam32;
	case 64: return tcpam64;
	case 128: return tcpam128;
	default:
		break;
	}
	return err_tcpam;
}


char *
power2string(power_t a)
{
	static char buf[64];
	switch( a ){
	case noPower:
		strcpy(buf,"off");
		break;
	case annex_b:
		strcpy(buf,"on");
		break;
	default:
		strcpy(buf,"pwr_err");
		break;
	}
	return buf;
}

power_t
string2power(char *a)
{
	if( !strncasecmp("on",a,2) )
		return powerFeed;
	if( !strncasecmp("off",a,3) )
		return noPower;
	return err_power;
}


//--------------- Normal mode output ----------

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
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u) moni(%d)\n",
		   p->cntrs.es,p->cntrs.ses,p->cntrs.crc,p->cntrs.losws,
		   p->cntrs.uas,p->cntrs.mon_sec);
    return 0;
}


int
print_endp_cur(app_comm_cli &cli,char *chan,unit u,side s,int loop)
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
    cli.send(req->frame_ptr(),req->frame_size());
    cli.wait();
    size = cli.recv(b);
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


//--------------- Shell mode output ----------
void shell_error(){
	printf("eoc_error=1\n");
}

int
shell_spanconf(app_comm_cli &cli,char *chan,int type)
{
    char *b;
    int size;
    // Endpoint current
    app_frame *req=NULL, *resp=NULL;
	int ret = 0;
    req = new app_frame(APP_SPAN_CONF,APP_GET,app_frame::REQUEST,1,chan);
    cli.send(req->frame_ptr(),req->frame_size());
    cli.wait();
    size = cli.recv(b);
	if( size<=0 ){
		goto exit;
	}
    resp = new app_frame(b,size);
    if( !resp->frame_ptr() ){
		ret = -1;
		goto exit;
    }
    if( resp->is_negative() ){ // no such unit or no net_side
		ret = -1;
    }else{
		span_conf_payload *p1 = (span_conf_payload*)resp->payload_ptr();
		if (type){
			printf("adm_reg_num=%d\nnaprof=%s\n",p1->nreps,p1->alarm_prof);
		}
		printf("type=%s\ncprof=\"%s\"\n",(p1->type==master) ? "master" : "slave",p1->conf_prof);
    }
 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
    return ret;
}

int
shell_spanstat(app_comm_cli &cli,char *chan)
{
    char *b;
    int size;
    // Endpoint current
    app_frame *req=NULL, *resp=NULL;
	int ret = 0;
    req = new app_frame(APP_SPAN_STATUS,APP_GET,app_frame::REQUEST,1,chan);
    cli.send(req->frame_ptr(),req->frame_size());
    cli.wait();
    size = cli.recv(b);
	if( size<=0 ){
		goto exit;
	}

    resp = new app_frame(b,size);
    if( !resp->frame_ptr() ){
		ret = -1;
		goto exit;
    }
    if( resp->is_negative() ){ // no such unit or no net_side
		ret = -1;
    }else{
		span_status_payload *p1 = (span_status_payload*)resp->payload_ptr();
		annex_t annex = annex_a;
		if( p1->region1 )
			annex = annex_b;
		printf("reg_num=\"%d\"\nrate=\"%d\"\nannex=\"%s\"\ntcpam=\"%s\"\n",p1->nreps,p1->max_lrate,
				annex2string(annex),tcpam2STRING((tcpam_t)p1->tcpam));

    }
 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
    return ret;

}


int
shell_channel(app_comm_cli &cli,char *chan,span_params_payload *p)
{
	printf("chan=\"%s\"\nunit_num=\"%d\"\nlink=\"%d\"\nloop_num=\"%d\"\n",chan,p->units,p->link_establ,p->loops);
	shell_spanconf(cli,chan);
	shell_spanstat(cli,chan);
}

int
shell_endp_cur(app_comm_cli &cli,char *chan,unit u,side s,int loop)
{
    char *b;
    int size;
	char date[256];
    // Endpoint current
    endp_cur_payload *p,*p1;
    app_frame *req=NULL, *resp=NULL;
	int ret = 0;


    req = new app_frame(APP_ENDP_CUR,APP_GET,app_frame::REQUEST,1,chan);
    p = (endp_cur_payload*)req->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = loop;
    cli.send(req->frame_ptr(),req->frame_size());
    cli.wait();
    size = cli.recv(b);
	if( size<=0 ){
		goto exit;
	}
    resp = new app_frame(b,size);
    if( !resp->frame_ptr() ){
		ret = -1;
		goto exit;
    }
    if( resp->is_negative() ){ // no such unit or no net_side
		ret = -1;
    }else{
		p1 = (endp_cur_payload*)resp->payload_ptr();
		if( p1->unit != p->unit || p1->side != p->side || p1->loop != p->loop ){
			ret = -1;
			goto exit;
		}
		printf("unit=\"%s\"\nside=\"%s\"\nloop=\"%d\"\n",unit2string((unit)p->unit),side2string((side)p->side),p->loop);
		printf("snr=\"%d\"\n",p1->cur_snr);
		printf("lattn=\"%d\"\n",p1->cur_attn);
		printf("es=\"%u\"\nses=\"%u\"\ncrc=\"%d\"\nlosws=\"%u\"\nuas=\"%u\"\n",
		   p1->total.es,p1->total.ses,p1->total.crc,p1->total.losws,p1->total.uas);

		strftime(date,256,"%d %b %G",localtime(&p1->relative_ts));
		printf("tdate=\"%s\"\n",date);
		strftime(date,256,"%R",localtime(&p1->relative_ts));
		printf("ttime=\"%s\"\n",date);
		printf("tes=\"%u\"\ntses=\"%u\"\ntcrc=\"%d\"\ntlosws=\"%u\"\ntuas=\"%u\"\n",
			   p1->relative.es,p1->relative.ses,p1->relative.crc,p1->relative.losws,p1->relative.uas);
		printf("m15es=\"%u\"\nm15ses=\"%u\"\nm15crc=\"%d\"\nm15losws=\"%u\"\nm15uas=\"%u\"\nm15monsec=\"%d\"\nm15elaps=\"%02dm:%02ds\"\n",
			   p1->cur15min.es,p1->cur15min.ses,p1->cur15min.crc,p1->cur15min.losws,p1->cur15min.uas,p1->cur15min.mon_sec,
			   (p1->cur_15m_elaps%(60*60))/60,p1->cur_15m_elaps%60);
		printf("d1es=\"%u\"\nd1ses=\"%u\"\nd1crc=\"%d\"\nd1losws=\"%u\"\nd1uas=\"%u\"\nd1monsec=\"%d\"\nd1elaps=\"%02dh:%02dm:%02ds\"\n",
			   p1->cur1day.es,p1->cur1day.ses,p1->cur1day.crc,p1->cur1day.losws,p1->cur1day.uas,p1->cur1day.mon_sec,
			   p1->cur_1d_elaps/(60*60),(p1->cur_1d_elaps%(60*60))/60,p1->cur_1d_elaps%60);
    }
 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
    return ret;
}


int
shell_endp_15m(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum)
{
    char *b;
    int size;
    // Endpoint current
    endp_15min_payload *p,*p1;
    app_frame *fr, *fr1;
	int ret = 0;

    fr = new app_frame(APP_ENDP_15MIN,APP_GET_NEXT,app_frame::REQUEST,1,chan);
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
		ret = -1;
		goto exit;
    }

    if( fr1->is_negative() ){ // no such unit or no net_side
		ret = -1;
		goto exit;
    } else {
		p1 = (endp_15min_payload*)fr1->payload_ptr();
		if( p1->unit != p->unit || p1->side != p->side
			|| p1->loop != p->loop ){
			ret = -1;
		}
		printf("unit=\"%s\"\nside=\"%s\"\nloop=\"%d\"\n",unit2string((unit)p1->unit),side2string((side)p1->side),p1->loop);
		printf("int=\"%d\"\n",p1->int_num);
		// Human readable time
		char s[256];
		time_t tm = time(NULL);
		int int_offs = p1->int_num;
		if( tm%(15*60) )
			int_offs--;
		tm -= tm%(15*60);
		tm -= 15*60*int_offs;
		strftime(s,256,"%d %b %G",localtime(&tm));
		printf("int_day=\"%s\"\n",s);
		strftime(s,256,"%R",localtime(&tm));
		printf("time_end=\"%s\"\n",s);
		tm -= 15*60;
		strftime(s,256,"%R",localtime(&tm));
		printf("time_start=\"%s\"\n",s);
		// Statistics
		float percent = ((float)p1->cntrs.mon_sec/(15*60))*100; // Percentage of day
		printf("es=\"%u\"\nses=\"%u\"\ncrc=\"%d\"\nlosws=\"%u\"\nuas=\"%u\"\nmon_sec=\"%d\"\nmon_pers=\"%.2f\"\n",
			   p1->cntrs.es,p1->cntrs.ses,p1->cntrs.crc,p1->cntrs.losws,p1->cntrs.uas,
			   p1->cntrs.mon_sec,percent);
    }
 exit:
	if( fr )
		delete fr;
	if( fr1 )
		delete fr1;
    return ret;
}


int
shell_endp_1d(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum)
{
    char *b;
    int size;
    // Endpoint current
    endp_1day_payload *p,*p1;
    app_frame *fr, *fr1;
	int ret = 0;

    fr = new app_frame(APP_ENDP_1DAY,APP_GET_NEXT,app_frame::REQUEST,1,chan);
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
		//		printf("No response\n");
		ret = -1;
		goto exit;
    }
    if( fr1->is_negative() ){ // no such unit or no net_side
		//		printf("Negative response\n");
		ret = -1;
		goto exit;
    } else {
		p1 = (endp_1day_payload*)fr1->payload_ptr();
		if( p1->unit != p->unit || p1->side != p->side
			|| p1->loop != p->loop ){
			//			printf("Error response\n");
			ret = -1;
			goto exit;
		}
		printf("unit=\"%s\"\nside=\"%s\"\nloop=\"%d\"\n",unit2string((unit)p1->unit),side2string((side)p1->side),p1->loop);
		printf("int=\"%d\"\n",p1->int_num);
		// Human readable time
		char s[256];
		time_t tm = time(NULL);
		int int_offs = p1->int_num;
		if( tm%(24*60*60) )
			int_offs--;
		tm -= tm%(24*60*60) + int_offs*24*60*60;
		strftime(s,256,"%d %b %G",localtime(&tm));
		printf("int_day=\"%s\"\n",s);
		// Statistics
		float percent = ((float)p1->cntrs.mon_sec/(24*60*60))*100; // Percentage of day
		printf("es=\"%u\"\nses=\"%u\"\ncrc=\"%d\"\nlosws=\"%u\"\nuas=\"%u\"\nmon_sec=\"%d\"\nmon_pers=\"%.2f\"\n",
			   p1->cntrs.es,p1->cntrs.ses,p1->cntrs.crc,p1->cntrs.losws,p1->cntrs.uas,
			   p1->cntrs.mon_sec,percent);
    }
 exit:
	if( fr )
		delete fr;
	if( fr1 )
		delete fr1;
    return ret;
}


int shell_cprof_list(app_comm_cli &cli)
{
    app_frame *req = NULL, *resp = NULL;
    char *buf;
    int flag = 0;
	char *profiles[256];
	int profiles_num = 0;
    cprof_list_payload *p=NULL,*p1=NULL;
	int ret = 0;

	req = new app_frame(APP_LIST_CPROF,APP_GET,app_frame::REQUEST,1,"");
	if( !req ){
		ret = -1;
		goto exit;
	}

    p = (cprof_list_payload*)req->payload_ptr();
	p->pname[0][0]='\0';

    do{
		p1 = (cprof_list_payload*)req->payload_ptr();
        cli.send(req->frame_ptr(),req->frame_size());
		cli.wait();
		int size = cli.recv(buf);
		if( size<=0 ){
			ret = -1;
			break;
		}

        resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			ret = -1;
			goto exit;
		}
		if( resp->is_negative() ){
			ret = -1;
			goto exit;
		}

        p = (cprof_list_payload*)resp->payload_ptr();
		for(int i=0;i<p->filled;i++){
			profiles[profiles_num++] = strdup(p->pname[i]);
		}

		if( !p->filled )
			break;
		flag = !p->last_msg;
        p1 = (cprof_list_payload*)req->payload_ptr();
		strncpy(p1->pname[0],p->pname[p->filled-1],SNMP_ADMIN_LEN);
		delete resp;
		resp = NULL;
    }while( flag );

	printf("cprof_list=\"");
	for(int i=0;i<profiles_num;i++){
		printf("%s ",profiles[i]);
		free(profiles[i]);
	}
	printf("\"\n");
 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return ret;
}


int shell_cprof_info(app_comm_cli &cli,char *prof,int ind)
{
    char *b;
    int size;
    // Endpoint current
	cprof_payload *p;
    app_frame *fr, *fr1;
	int ret = 0;

    fr = new app_frame(APP_CPROF,APP_GET,app_frame::REQUEST,1,"");
    p = (cprof_payload*)fr->payload_ptr();
    strncpy(p->pname,prof,SNMP_ADMIN_LEN);
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);

    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
		ret = -1;
		goto exit;
    }
    if( fr1->is_negative() ){ // no such unit or no net_side
		ret = -1;
		goto exit;
    } else {

// 		p = (cprof_payload*)fr1->payload_ptr();
// 		printf("cprof=%s\n",p->pname);
// 		printf("annex=%s\npower=%s\nrate=%d\n",
// 			   annex2string(p->conf.annex),power2string(p->conf.power),
// 			   p->conf.rate);

		p = (cprof_payload*)fr1->payload_ptr();
		char buf[256];
		EOC_dev::comp_name(p->comp,buf);
		if( ind ){
			printf("cprof%d=\"%s\"\n",ind,p->pname);
			printf("annex%d=\"%s\"\npower%d=\"%s\"\nrate%d=\"%d\"\ntcpam%d=\"%s\"\ncompat%d=\"%s\"\n",
				   ind,annex2string(p->conf.annex),ind,power2string(p->conf.power),
				   ind,p->conf.rate,ind,tcpam2string(p->conf.tcpam),ind,buf);
		}else{
			printf("cprof=\"%s\"\n",p->pname);
			printf("annex=\"%s\"\npower=\"%s\"\nrate=\"%d\"\ntcpam=\"%s\"\ncompat%d=\"%s\"\n",
				   annex2string(p->conf.annex),power2string(p->conf.power),
				   p->conf.rate,tcpam2string(p->conf.tcpam),ind,buf);
		}
    }
exit:
	if( fr )
		delete fr;
	if( fr1 )
		delete fr1;
    return ret;
}


int shell_cprof_full(app_comm_cli &cli)
{
    app_frame *req = NULL, *resp = NULL;
    char *buf;
    int flag = 0;
	int profiles_num = 0;
    cprof_list_payload *p=NULL,*p1=NULL;
	int ret = 0;

	req = new app_frame(APP_LIST_CPROF,APP_GET,app_frame::REQUEST,1,"");
	if( !req ){
		ret = -1;
		goto exit;
	}

    p = (cprof_list_payload*)req->payload_ptr();
	p->pname[0][0]='\0';

    do{
		p1 = (cprof_list_payload*)req->payload_ptr();
        cli.send(req->frame_ptr(),req->frame_size());
		cli.wait();
		int size = cli.recv(buf);
		if( size<=0 ){
			ret = -1;
			break;
		}

        resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			ret = -1;
			goto exit;
		}
		if( resp->is_negative() ){
			ret = -1;
			goto exit;
		}

        p = (cprof_list_payload*)resp->payload_ptr();
		for(int i=0;i<p->filled;i++){
			shell_cprof_info(cli,p->pname[i],++profiles_num);
		}

		if( !p->filled )
			break;
		flag = !p->last_msg;
        p1 = (cprof_list_payload*)req->payload_ptr();
		strncpy(p1->pname[0],p->pname[p->filled-1],SNMP_ADMIN_LEN);
		delete resp;
		resp = NULL;
    }while( flag );

 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return ret;
}



