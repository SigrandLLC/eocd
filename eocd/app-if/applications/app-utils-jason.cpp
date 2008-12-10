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
#include <app-if/err_strings.h>
#include "app-utils.h"

int fd[2] = {-1,-1};
FILE *out,*in;

#define bprintf(fmt,args...) {\
	fprintf(out,fmt,## args); \
}




//------------- JASON error handling. Print directly on stdout -------------//
void do_indent_dir(int indent){
	for(int i=0;i<indent;i++){
		printf("    ");
	}
}

// 
void jason_error(int ret,int indent){
	do_indent_dir(indent);
	printf("{\n");
	do_indent_dir(indent+1);
	printf("\"eoc_error\": \"1\",\n");
	do_indent_dir(indent+1);
	printf("\"err_srting\" : \"(%d) %s\"\n",ret,err_strings[ret-1]);
	do_indent_dir(indent);
	printf("}");
}

void jason_error(char *s,int indent){
	do_indent_dir(indent);
	printf("{\n");
	do_indent_dir(indent+1);
	printf("\"eoc_error\": \"1\"\n");
	do_indent_dir(indent+1);
	printf("\"err_srting\" : \"%s\"\n",s);
	do_indent_dir(indent);
	printf("}");
}


//------------------ JASON payload. Print into temporary buffer. 
// In error case ignore it, on success - print on stdout-----//


//---------- Temproary pipe - output temporary storage --------------//
int fd_set_nonblock(int fd,int nonblock)
{
	int flags;
	char err_str[256];
	
	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) {
		sprintf(err_str,"I/O error: fcntl(%d, F_GETFL) failed", fd);
		jason_error(err_str);
		return -1;
	}

	if (nonblock)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		sprintf(err_str,"I/O error: fcntl(%d, F_SETFL) failed", fd);
		jason_error(err_str);
		return -1;
	}
	return 0;
}


int jason_init()
{
	if( pipe(fd) ){
		return -1;
	}
	if( fd_set_nonblock(fd[0],1) )
		return -1;
	in = fdopen(fd[0],"r");
	out = fdopen(fd[1],"w");
	return 0;
}

int jason_flush()
{
	int buf[512];
	int count;
	int i = 0;

	fclose(out);

	while( (count = fread(buf,1,510,in)) > 0 ){
		write(1,buf,count);
	}
}

void do_indent(int indent){
	for(int i=0;i<indent;i++){
		bprintf("    ");
	}
}

void jason_sensor(int indent,int snum,int cur,int cnt)
{
	do_indent(indent);
	bprintf("{\n");
	do_indent(indent+1);
	bprintf("\"num\" : \"%d\",",snum);
	do_indent(indent+1);
	bprintf("\"cur\" : \"%d\",",cur);
	do_indent(indent+1);
	bprintf("\"cnt\" : \"%d\"",cur);
	do_indent(indent);
	bprintf("}");
}

int jason_sensors(int indent,app_comm_cli &cli,char *chan,unit u)
{
    app_frame *req = new app_frame(APP_SENSORS,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
	int ret = 0;
	
	((sensors_payload *)req->payload_ptr())->unit = u;
    cli.send(req->frame_ptr(),req->frame_size());
	cli.wait();
    int size = cli.recv(buf);
    if( size <=0 ){
		delete req;
		char err_str[256];
		sprintf(err_str,"Error: receive sensor info for unit: %d",u);
		jason_error(err_str);
		ret = -1;
		goto err_exit;
    }
    
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
		char err_str[256];
		sprintf(err_str,"Error: bad message from eocd for sensors of unit: %d",u);
		jason_error(err_str);
		ret = -1;
		goto err_exit;
    } 
    
    if( resp->is_negative() ){ // no such unit or no net_side
		char err_str[256];
		sprintf(err_str,"Error: negative response from server for sensors of unit: %d",u);
		jason_error(err_str);
		ret = -1;
		goto err_exit;
    }
    {
		sensors_payload *p = (sensors_payload *)resp->payload_ptr();
		jason_sensor(indent,1,p->state.sensor1,p->sens1);
		bprintf(",\n");
		jason_sensor(indent,2,p->state.sensor2,p->sens2);
		bprintf(",\n");
		jason_sensor(indent,3,p->state.sensor3,p->sens3);
		bprintf("\n");
    }
 err_exit:
    delete resp;
    delete req;
    return ret;
}

void jason_pbo(int indent,int mode,char *val)
{
	do_indent(indent);
	bprintf("\"pbo\" : {\n");
	do_indent(indent+1);
	bprintf("\"mode\" : \"%d\",",mode);
	do_indent(indent+1);
	bprintf("\"val\" : \"%s\"",val);
	do_indent(indent);
	bprintf("}");
}


void jason_short_channel(int indent,struct eoc_channel *chan)
{
	do_indent(indent);
	bprintf("{\n");
	do_indent(indent+1);
	bprintf("\"name\" : \"%s\",\n",chan->name);
	do_indent(indent+1);
	bprintf("\"type\" : \"%s\"\n",chan->t == slave ? "slave" : "master");
	do_indent(indent);
	bprintf("}");
}

int
jason_channels_list(struct eoc_channel *channels,int cnum)
{
	int indent = 0;
	int ret = 0;

	do_indent(indent);
	bprintf("{\n");

	do_indent(indent+1);
	bprintf("\"channels\" : [\n");
	for(int i=0;i<cnum;i++){
		jason_short_channel(indent+2,channels+i);
		if( i < cnum-1 ){
			do_indent(indent);
			bprintf(",\n");
		}
		free(channels[i].name);
	}
	bprintf("\n");
	do_indent(indent+1);
	bprintf("]\n");
	do_indent(indent);
	bprintf("}\n");
	
	return ret;
}


int
jason_spanconf(int indent,app_comm_cli &cli,char *chan)
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
		jason_error("Error while receive message from server");
		ret = -1;
		goto exit;
    } 
    if( (ret = resp->is_negative()) ){ // no such unit or no net_side
		jason_error(ret);
		ret = -ret;
		goto exit;
    }else{
		span_conf_payload *p1 = (span_conf_payload*)resp->payload_ptr();
		do_indent(indent);
		bprintf("\"conf\" : {\n");
		if( p1->type == master ){
			do_indent(indent+1);
			bprintf("\"adm_reg_num\" : \"%d\",\n",p1->nreps);
			do_indent(indent+1);
			bprintf("\"naprof\" : \"%s\",\n",p1->alarm_prof);
		}
		do_indent(indent+1);
		bprintf("\"type\" : \"%s\",\n",(p1->type==master) ? "master" : "slave");
		do_indent(indent+1);
		bprintf("\"cprof\" : \"%s\"\n",p1->conf_prof);

		do_indent(indent);
		bprintf("}");
    }
 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
    return ret;
}

int
jason_spanstat(int indent,app_comm_cli &cli,char *chan)
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
		jason_error("Error while receive message from server");
		ret = -1;
		goto exit;
    } 
    if( (ret=resp->is_negative()) ){ // no such unit or no net_side
		jason_error(ret);
		ret = -ret;
		goto exit;
    }else{
		span_status_payload *p1 = (span_status_payload*)resp->payload_ptr();
		annex_t annex = annex_a;
		if( p1->region1 )
			annex = annex_b;
		
		do_indent(indent);
		bprintf("\"status\" : {\n");
		do_indent(indent+1);
		bprintf("\"reg_num\" : \"%d\",\n",p1->nreps);
		do_indent(indent+1);
		bprintf("\"rate\" : \"%d\",\n",p1->max_lrate);
		do_indent(indent+1);
		bprintf("\"annex\" : \"%s\",\n",annex2string(annex));
		do_indent(indent+1);
		bprintf("\"annex\" : \"%s\",\n",tcpam2STRING((tcpam_t)p1->tcpam));
		do_indent(indent);
		bprintf("}");
    }
 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
    return ret;

}


int
jason_channel(int indent,app_comm_cli &cli,char *chan,span_params_payload *p)
{
	do_indent(indent);
	bprintf("\"%s\" : {\n",chan);
	do_indent(indent+1);
	bprintf("\"unit_num\" : \"%d\",\n",p->units);
	do_indent(indent+1);
	bprintf("\"link\" : \"%d\",\n",p->link_establ);
	do_indent(indent+1);
	bprintf("\"loop_num\" : \"%d\",\n",p->loops);
	jason_spanconf(indent+1,cli,chan);
	bprintf(",\n");
	jason_spanstat(indent+1,cli,chan);
	bprintf("\n");
	do_indent(indent);
	bprintf("}");
	return 0;
}

int jason_m15ints(int indent,app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum)
{
    char *b;
    int size;
	int offset = 0;
    // Endpoint current
    endp_15min_payload *p,*p1;
    app_frame *fr, *fr1;
	int ret = 0;
	int start_int, end_int;

	if( inum == -1 ){
		start_int = 1;
		end_int = 96;
	}else{
		start_int = inum;
		end_int = inum;
	}
	
	for(inum = start_int;inum <= end_int;inum++){
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
			jason_error("Error while receive message from server");
			ret = -1;
			goto exit;
	    } 
	
    	if( ret=fr1->is_negative() ){ // no such unit or no net_side
			break;
	    } else {
			p1 = (endp_15min_payload*)fr1->payload_ptr();
			if( p1->unit != p->unit || p1->side != p->side || p1->loop != p->loop ){
				char str[256];
				sprintf(str,"15-minutes int: No information about unit=%d, int=%d aviliable",u,inum);
				jason_error(str);
				ret = -1;
				break;
			}
			
			if( start_int == end_int && p1->int_num != p->int_num ){
				char str[256];
				sprintf(str,"15-minutes int: No information about unit=%d, int=%d aviliable",u,inum);
				jason_error(str);
				ret = -1;
				break;
			}else{
				inum = p1->int_num;
			}
			
			do_indent(indent+offset);
			bprintf("{\n");
			offset++;
			
			do_indent(indent+offset);
			bprintf("\"int\" : \"%d\",\n",p1->int_num);	
			
			// Human readable time
			char s[256];
			time_t tm = time(NULL);
			int int_offs = p1->int_num;
			if( tm%(15*60) )
				int_offs--;
			tm -= tm%(15*60);
			tm -= 15*60*int_offs;
			strftime(s,256,"%d %b %G",localtime(&tm));                                  
			do_indent(indent+offset);
			bprintf("\"int_day\" : \"%s\",\n",s);

			strftime(s,256,"%R",localtime(&tm));
			do_indent(indent+offset);
			bprintf("\"time_end\" : \"%s\",\n",s);

			tm -= 15*60;
			strftime(s,256,"%R",localtime(&tm));
			do_indent(indent+offset);
			bprintf("\"time_start\" : \"%s\",\n",s);             

			// Statistics
			float percent = ((float)p1->cntrs.mon_sec/(15*60))*100; // Percentage of day

			do_indent(indent+offset);
			bprintf("\"es\" : \"%u\",\n",p1->cntrs.es);
			do_indent(indent+offset);
			bprintf("\"ses\" : \"%u\",\n",p1->cntrs.ses);
			do_indent(indent+offset);
			bprintf("\"crc\" : \"%d\",\n",p1->cntrs.crc);
			do_indent(indent+offset);
			bprintf("\"losws\" : \"%u\",\n",p1->cntrs.losws);
			do_indent(indent+offset);
			bprintf("\"uas\" : \"%u\",\n",p1->cntrs.uas);
			do_indent(indent+offset);
			bprintf("\"mon_sec\" : \"%d\",\n",p1->cntrs.mon_sec);
			do_indent(indent+offset);
			bprintf("\"mon_pers\" : \"%.2f\"\n",percent);

			offset--;
			do_indent(indent+offset);
			bprintf("}");
			if( inum < end_int ){
				bprintf(",\n");
			}
	    }
	}
	
	if( start_int == end_int ) {
		// Requested exact interval
		if( ret > 0 ){
			jason_error(ret);
			ret = -ret;
		}
	}else{
		ret = 0;
	}
 exit:
	if( fr )
		delete fr;
	if( fr1 )
		delete fr1;
    return ret;
}


int jason_d1ints(int indent,app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum)
{
    char *b;
    int size;
	int offset = 0;
    // Endpoint current
    endp_1day_payload *p,*p1;
    app_frame *fr, *fr1;
	int ret = 0;
	int start_int, end_int;

	if( inum == -1 ){
		start_int = 1;
		end_int = 30;
	}else{
		start_int = inum;
		end_int = inum;
	}
	
	for(inum = start_int;inum <= end_int;inum++){
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
			jason_error("Error while receive message from server");
			ret = -1;
			goto exit;
	    } 
	
    	if( ret=fr1->is_negative() ){ // no such unit or no net_side
			break;
	    } else {
			p1 = (endp_1day_payload*)fr1->payload_ptr();
			if( p1->unit != p->unit || p1->side != p->side || p1->loop != p->loop ){
				char str[256];
				sprintf(str,"1-day int: No information about unit=%d, int=%d aviliable",u,inum);
				jason_error(str);
				ret = -1;
				break;
			}
			
			if( start_int == end_int && p1->int_num != p->int_num ){
				char str[256];
				sprintf(str,"1-day int: No information about unit=%d, int=%d aviliable",u,inum);
				jason_error(str);
				ret = -1;
				break;
			}else{
				inum = p1->int_num;
			}
			
			do_indent(indent+offset);
			bprintf("{\n");
			offset++;
			
			do_indent(indent+offset);
			bprintf("\"int\" : \"%d\",\n",p1->int_num);	
			
			// Human readable time
			char s[256];
			time_t tm = time(NULL);
			int int_offs = p1->int_num;
			if( tm%(24*60*60) )
				int_offs--;
			tm -= tm%(24*60*60) + int_offs*24*60*60;
			strftime(s,256,"%d %b %G",localtime(&tm));                                  
			do_indent(indent+offset);
			bprintf("\"int_day\" : \"%s\",\n",s);

			// Statistics
			float percent = ((float)p1->cntrs.mon_sec/(24*60*60))*100; // Percentage of day

			do_indent(indent+offset);
			bprintf("\"es\" : \"%u\",\n",p1->cntrs.es);
			do_indent(indent+offset);
			bprintf("\"ses\" : \"%u\",\n",p1->cntrs.ses);
			do_indent(indent+offset);
			bprintf("\"crc\" : \"%d\",\n",p1->cntrs.crc);
			do_indent(indent+offset);
			bprintf("\"losws\" : \"%u\",\n",p1->cntrs.losws);
			do_indent(indent+offset);
			bprintf("\"uas\" : \"%u\",\n",p1->cntrs.uas);
			do_indent(indent+offset);
			bprintf("\"mon_sec\" : \"%d\",\n",p1->cntrs.mon_sec);
			do_indent(indent+offset);
			bprintf("\"mon_pers\" : \"%.2f\"\n",percent);

			offset--;
			do_indent(indent+offset);
			bprintf("}");
			if( inum < end_int ){
				bprintf(",\n");
			}
	    }
	}
	
	if( start_int == end_int ) {
		// Requested exact interval
		if( ret > 0 ){
			jason_error(ret);
			ret = -ret;
		}
	}else{
		ret = 0;
	}


 exit:
	if( fr )
		delete fr;
	if( fr1 )
		delete fr1;
    return ret;
}


int
jason_loop(int indent,app_comm_cli &cli,char *chan,unit u,side s,int loop)
{
    char *b;
    int size;
	char date[256];
    // Endpoint current
    endp_cur_payload *p,*p1;
    app_frame *req=NULL, *resp=NULL;
	int ret = 0;
	int offset = 0;


	// 
    req = new app_frame(APP_ENDP_CUR,APP_GET,app_frame::REQUEST,1,chan);
    p = (endp_cur_payload*)req->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = loop;
    cli.send(req->frame_ptr(),req->frame_size());
    cli.wait();
    size = cli.recv(b);
	if( size<=0 ){
		ret = -1;
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
		
		do_indent(indent + offset);
		bprintf("{\n");
		offset++;

		do_indent(indent + offset);
		bprintf("\"name\" : \"loop%d\",\n",loop);
		// Current counters
		do_indent(indent + offset);
		bprintf("\"cur\" : {\n");
		offset++;

		do_indent(indent + offset);
		bprintf("\"snr\" : \"%d\",\n",p1->cur_snr);
		do_indent(indent + offset);
		bprintf("\"lattn\" : \"%d\",\n",p1->cur_attn);
		do_indent(indent + offset);
		bprintf("\"es\" : \"%u\",\n",p1->total.es);
		do_indent(indent + offset);
		bprintf("\"ses\" : \"%u\",\n",p1->total.ses);
		do_indent(indent + offset);
		bprintf("\"crc\" : \"%d\",\n",p1->total.crc);
		do_indent(indent + offset);
		bprintf("\"losws\" : \"%u\",\n",p1->total.losws);
		do_indent(indent + offset);
		bprintf("\"uas\" : \"%u\",\n",p1->total.uas);

		// current 15 minute interval counters
		do_indent(indent + offset);
		bprintf("\"m15int\" : {\n");
		offset++;

		do_indent(indent + offset);
		bprintf("\"es\" : \"%u\",\n",p1->cur15min.es);
		do_indent(indent + offset);
		bprintf("\"ses\" : \"%u\",\n",p1->cur15min.ses);
		do_indent(indent + offset);
		bprintf("\"crc\" : \"%d\",\n",p1->cur15min.crc);
		do_indent(indent + offset);
		bprintf("\"losws\" : \"%u\",\n",p1->cur15min.losws);
		do_indent(indent + offset);
		bprintf("\"uas\" : \"%u\",\n",p1->cur15min.uas);
		do_indent(indent + offset);
		bprintf("\"mon_sec\" : \"%u\",\n",p1->cur15min.mon_sec);
		do_indent(indent + offset);
		bprintf("\"elapsed\" : \"%02dm:%02ds\"\n",(p1->cur_15m_elaps%(60*60))/60,p1->cur_15m_elaps%60);

		offset--;
		do_indent(indent + offset);
		bprintf("},\n");

		// current 1 day interval counters
		do_indent(indent + offset);
		bprintf("\"d1int\" : {\n");
		offset++;

		do_indent(indent + offset);
		bprintf("\"es\" : \"%u\",\n",p1->cur1day.es);
		do_indent(indent + offset);
		bprintf("\"ses\" : \"%u\",\n",p1->cur1day.ses);
		do_indent(indent + offset);
		bprintf("\"crc\" : \"%d\",\n",p1->cur1day.crc);
		do_indent(indent + offset);
		bprintf("\"losws\" : \"%u\",\n",p1->cur1day.losws);
		do_indent(indent + offset);
		bprintf("\"uas\" : \"%u\",\n",p1->cur1day.uas);
		do_indent(indent + offset);
		bprintf("\"mon_sec\" : \"%u\",\n",p1->cur1day.mon_sec);
		do_indent(indent + offset);
		bprintf("\"elapsed\" : \"%02dh:%02dm:%02ds\"\n",
				p1->cur_1d_elaps/(60*60),(p1->cur_1d_elaps%(60*60))/60,p1->cur_1d_elaps%60);

		offset--;
		do_indent(indent + offset);
		bprintf("}\n");

		offset--;
		do_indent(indent + offset);
		bprintf("},\n");

		// Relative counters
		do_indent(indent + offset);
		bprintf("\"rel\" : {\n");
		offset++;

		do_indent(indent + offset);
		strftime(date,256,"%d %b %G",localtime(&p1->relative_ts));
		bprintf("\"date\" : \"%s\",\n",date);
		do_indent(indent + offset);
		strftime(date,256,"%R",localtime(&p1->relative_ts));
		bprintf("\"time\" : \"%s\",\n",date);
		do_indent(indent + offset);
		bprintf("\"es\" : \"%u\",\n",p1->relative.es);
		do_indent(indent + offset);
		bprintf("\"ses\" : \"%u\",\n",p1->relative.ses);
		do_indent(indent + offset);
		bprintf("\"crc\" : \"%d\",\n",p1->relative.crc);
		do_indent(indent + offset);
		bprintf("\"losws\" : \"%u\",\n",p1->relative.losws);
		do_indent(indent + offset);
		bprintf("\"uas\" : \"%u\"\n",p1->relative.uas);

		offset--;
		do_indent(indent + offset);
		bprintf("},\n");


		// 15-min intervals
		do_indent(indent + offset);
		bprintf("\"m15int\" : [\n");
		if( (ret = jason_m15ints(indent+offset+1,cli,chan,u,s,loop)) ){
			goto exit;
		}
		bprintf("\n");
		do_indent(indent + offset);
		bprintf("],\n");

		// 1-day intervals
		do_indent(indent + offset);
		bprintf("\"d1int\" : [\n");
		if( (ret = jason_d1ints(indent+offset+1,cli,chan,u,s,loop)) )
			goto exit;
		bprintf("\n");
		do_indent(indent + offset);
		bprintf("]\n");
    }
	offset--;
	do_indent(indent + offset);
	bprintf("}");
 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
    return ret;
}

int jason_side(int indent,app_comm_cli &cli,char *chan,span_params_payload *p,unit u,side s)
{
	int ret = 0;
	// Print out full info about unit
	do_indent(indent);
	bprintf("{\n");
	do_indent(indent+1);
	bprintf("\"name\" : \"%s\",\n",side2string(s));

	do_indent(indent+1);
	bprintf("\"loops\" : [\n"); 
	for(int loop=0;loop<p->loops;loop++){
		if( (ret=jason_loop(indent+2,cli,chan,u,s,loop)) ){
			return ret;
		}
		if( loop < p->loops-1 )
			bprintf(",\n");
	}
	bprintf("\n");
	do_indent(indent+1);
	bprintf("]\n");
	
	do_indent(indent);
	bprintf("}");
	
	return ret;
}

int
jason_unit(int indent,app_comm_cli &cli,char *chan,span_params_payload *p,unit u)
{
	int ret = 0;
	int offset = 0;
	
	do_indent(indent + offset);
	bprintf("\"%s\" : {\n",unit2string(u));
	offset++;

	// Unit sensors info. Only SRUx has sensors
	switch(u){
	case sru1:
	case sru2:
	case sru3:
	case sru4:
	case sru5:
	case sru6:
	case sru7:
	case sru8:
		// Sensors
		do_indent(indent + offset);
		bprintf("\"sensors\" : [\n");
		offset++;
		if( (ret = jason_sensors(indent+offset+1,cli,chan,u)) ){
			return ret;
		}
		bprintf("\n");
		offset--;
		do_indent(indent + offset);
		bprintf("],\n");
		break;
	}
	
	do_indent(indent + offset);
	bprintf("\"sides\" : [\n",unit2string(u));
	offset++;
	
	switch(u){
	case stu_c:
		ret = jason_side(indent+offset,cli,chan,p,u,cust_side);
		break;
	case stu_r:
		ret = jason_side(indent+offset,cli,chan,p,u,net_side);
		break;
	case sru1:
	case sru2:
	case sru3:
	case sru4:
	case sru5:
	case sru6:
	case sru7:
	case sru8:
		if( (ret=jason_side(indent+offset,cli,chan,p,u,cust_side) ) )
			break;
		bprintf(",\n");
		ret = jason_side(indent+offset,cli,chan,p,u,net_side);
		break;
	default:{
		goto err_exit;
	}
	}

	bprintf("\n");

	offset--;
	do_indent(indent + offset);
	bprintf("]\n");

	offset--;
	do_indent(indent + offset);
	bprintf("}\n");
	return ret;
err_exit:
	char errstr[256];
	sprintf(errstr,"Incorrect channel unit: %d",u);
	jason_error(errstr);
	return -1;
}

int jason_exact(int indent,app_comm_cli &cli,char *chan,unit _unit)
{
    app_frame *req = new app_frame(APP_SPAN_PARAMS,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp;
    char *buf;
	int ret = 0;
	int offset = 0;

	if( req == NULL ){
		jason_error("Error(jason_exact): Cannot allocate req");
		return -1;
	}

    cli.send(req->frame_ptr(),req->frame_size());
    cli.wait();
    int size = cli.recv(buf);
    if( !size ){
		jason_error("Error(jason_exact): Receive");
		delete req;
		return -1;
    }
    resp = new app_frame(buf,size);
    if( !resp->frame_ptr() ){
		jason_error("Bad message from server");
		ret = -1;
		goto exit;
    } 
    
    if( (ret=resp->is_negative()) ){ // no such unit or no net_side
		jason_error(ret);
		ret = -1;
		goto exit;
    }

	do_indent(indent+offset);
	printf("{\n");
	offset++;

	{
		span_params_payload *p = (span_params_payload *)resp->payload_ptr();
		if( _unit == unknown ){
			// Unit was not setted. Print out channel configuration 
			
			ret = jason_channel(indent+offset,cli,chan,p);
			goto exit;
		}else if( _unit > p->units ){
			// Unit bigger than aviliable
			char errstr[256];
			sprintf(errstr,"Incorrect channel unit: %d",_unit);
			jason_error(errstr);
			ret = -1;
			goto exit;
		}
		// Print full info about unit
		ret = jason_unit(indent+offset,cli,chan,p,_unit);
	}

 exit:
	offset--;
	do_indent(indent+offset);
	printf("}\n");
	
    delete resp;
    delete req;
    return ret;
}

int jason_cprof_info(int indent,app_comm_cli &cli,char *prof)
{
    char *b;
    int size;
    // Endpoint current
	cprof_payload *p;
    app_frame *fr, *fr1;
	int ret = 0;
	int offset = 0;
    fr = new app_frame(APP_CPROF,APP_GET,app_frame::REQUEST,1,"");
	if( !fr ){
		jason_error("Error(jason_cprof_info): Cannot allocate fr");
		return -1;
	}

    p = (cprof_payload*)fr->payload_ptr();
    strncpy(p->pname,prof,SNMP_ADMIN_LEN);
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
	if( size<=0 ){
		jason_error("Error(jason_cprof_info): size = 0");
		ret = -1;
		goto exit;
	}

    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
		jason_error("Error(jason_cprof_info): bad message");
		ret = -1;
		goto exit;
    } 
	
    if( ret = fr1->is_negative() ){ // no such unit or no net_side
		jason_error(ret);
		ret = -ret;
		goto exit;
    } else {
	
		p = (cprof_payload*)fr1->payload_ptr();
		
		do_indent(indent+offset);
		bprintf("{\n");
		offset++;
		
		do_indent(indent+offset);
		bprintf("\"name\" : \"%s\",\n",p->pname);
		do_indent(indent+offset);
		bprintf("\"annex\" : \"%s\",\n",annex2string(p->conf.annex));
		do_indent(indent+offset);
		bprintf("\"power\" : \"%s\",\n",power2string(p->conf.power));
		do_indent(indent+offset);
		bprintf("\"rate\" : \"%d\",\n",p->conf.rate);
		do_indent(indent+offset);
		bprintf("\"tcpam\" : \"%s\"\n",tcpam2string(p->conf.tcpam));
	
		offset--;
		do_indent(indent+offset);
		bprintf("}");
    }
exit:

	if( fr )
		delete fr;
	if( fr1 )
		delete fr1;
    return ret;
}


int jason_cprof_full(app_comm_cli &cli)
{
    app_frame *req = NULL, *resp = NULL;
    char *buf;
    int flag = 0;
	int profiles_num = 0;
    cprof_list_payload *p=NULL,*p1=NULL;
	int ret = 0;
	int indent = 0,offset=0;

	req = new app_frame(APP_LIST_CPROF,APP_GET,app_frame::REQUEST,1,"");
	if( !req ){
		jason_error("Error(jason_cprof_full): Cannot allocate req");
		return -1;
	}
	do_indent(indent+offset);
	bprintf("{\n");
	offset++;

	do_indent(indent+offset);
	bprintf("\"profiles\" : [\n");
	offset++;

    p = (cprof_list_payload*)req->payload_ptr();
	p->pname[0][0]='\0';

    do{
		p1 = (cprof_list_payload*)req->payload_ptr();
        cli.send(req->frame_ptr(),req->frame_size());
		cli.wait();
		int size = cli.recv(buf);
		if( size<=0 ){
			jason_error("Error(jason_cprof_full): size <= 0");
			ret = -1;
			break;
		}

        resp = new app_frame(buf,size);
		if( !resp->frame_ptr() ){
			jason_error("Error(jason_cprof_full): bad message");
			ret = -1;
			goto exit;
		} 
		if( ret = resp->is_negative() ){
			jason_error(ret);
			ret = -ret;
			goto exit;
		}

        p = (cprof_list_payload*)resp->payload_ptr();
		flag = !p->last_msg;
		for(int i=0;i<p->filled;i++){
			if( ret = jason_cprof_info(indent+offset,cli,p->pname[i]) ){
				goto exit;
			}
			if( flag || i<(p->filled-1) ){
				bprintf(",\n");
			}else{
				bprintf("\n");
			}
			++profiles_num;
		}	

		if( !p->filled )
			break;
        p1 = (cprof_list_payload*)req->payload_ptr();
		strncpy(p1->pname[0],p->pname[p->filled-1],SNMP_ADMIN_LEN);
		delete resp;
		resp = NULL;
    }while( flag );

	offset--;
	do_indent(indent+offset);
	bprintf("]\n");

	offset--;
	do_indent(indent+offset);
	bprintf("}\n");

 exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return ret;
}
