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
#include "app-utils-accum.h"

int
eocd_request(app_comm_cli &cli,app_frame *req,app_frame *&resp,output_t omode)
{
	char *buf;
	int size = 0, ret = 0;
	cli.send(req->frame_ptr(),req->frame_size());
	cli.wait();
	size = cli.recv(buf);
	if( size <= 0 ){
		print_error(omode,"fatal error: receive frame from eocd for channel \'%s\'",req->chan_name());
		exit(0);
	}

	resp = new app_frame(buf,size);
	if( !resp ){
		print_error(omode,"fatal error (%s): low on memory\n",req->chan_name());
		exit(0);
	}
	if( !resp->frame_ptr() ){
		print_error(omode,"fatal error (%s): bad message from eocd",req->chan_name());
		exit(0);
	}
	if( ret = resp->is_negative() ){ // no such unit or no net_side
		return ret;
	}
	return 0;
}

#define MAX_CHANNELS 64
int
accum_channel_list(app_comm_cli &cli,struct eoc_channel channels[MAX_CHANNELS],int &chan_cnt,output_t omode)
{
    app_frame *req = new app_frame(APP_SPAN_NAME,APP_GET,app_frame::REQUEST,1,"");
	app_frame *resp = NULL;
    char *buf;
    int flag = 0, ret = 0;

	chan_cnt = 0;
	
	if( !req ){
		print_error(omode,"fatal error: low on memory\n");
		exit(0);
	}

    do{
		if( (ret=eocd_request(cli,req,resp,omode)) ){
			// TODO: Maybe ret value check is needed
			break;
		}

		span_name_payload *p = (span_name_payload*)resp->payload_ptr();
		for(int i=0;i<p->filled && chan_cnt < MAX_CHANNELS;i++){
			channels[chan_cnt].name = strdup(p->spans[i].name);
			channels[chan_cnt].t = p->spans[i].t;
			channels[chan_cnt++].comp = p->spans[i].comp;
		}
		if( !p->filled )
			break;
		flag = !p->last_msg;
		req->chan_name(p->spans[p->filled-1].name);
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


int 
accum_ints(app_comm_cli &cli,channel_info_t &info,unit u,side s,output_t omode)
{
	endp_int_payload *p, *p1;
	app_frame *req = NULL, *resp = NULL;
	int start_int = 1, end_int;
	int size, snum = side2index(s,u,info), ret = 0;
	char *b;
	endp_int_payload ints[96];

	
	if( snum < 0 ){
		print_error(omode,"fatal error (%s): error side: %d\n",info.name,s);
		exit(0);
	}
	
	app_ids id;
	switch( info.tbl_type ){
	case INT15:
		id = APP_ENDP_15MIN;
		end_int = 96;
		break;
	case INT1D:
		id = APP_ENDP_1DAY;
		end_int = 30;
		break;
	default:
		return 0;
	}

	
	int index,inum;
	req = new app_frame(id, APP_GET_NEXT, app_frame::REQUEST,1,info.name);
	if( !req ){
		print_error(omode,"fatal error (%s): low on memory\n",info.name);
		exit(0);
	}
	for(inum = start_int, index = 0; inum <= end_int; inum++, index++) {
		p = (endp_int_payload*)req->payload_ptr();
		p->unit = u;
		p->side = s;
		p->loop = 0; // At this moment we have only one loop
		p->int_num = inum;
		
		if(ret = eocd_request(cli,req,resp,omode)) { 
			// no such unit or no net_side or no more non-zero intervals
			break;
		}
		p1 = (endp_int_payload*)resp->payload_ptr();
		if (p1->unit != p->unit || p1->side != p->side || p1->loop != p->loop) {
			print_error(omode,"fatal error (%s): get information about wrong unit:\n"
				"\trequested: %s.%s.%s.%d\n"
				"\tgetted:    %s.%s.%s.%d\n",
				info.name,
				info.name,unit2string(u),side2string(s),p->loop,
				info.name,unit2string((unit)p1->unit),side2string((side)p1->side),p1->loop);
			exit(0);
		}
		inum = p1->int_num;
		// Save incoming data
		ints[index] = *p1;
		if( resp )
			delete resp;
		resp = NULL;
	}

	switch( info.tbl_type ){
	case INT15:
		if( index ){
			info.units[(int)u -1].sints15m[snum] = new endp_int_payload[index]; 
			memcpy(info.units[(int)u -1].sints15m[snum],ints,sizeof(endp_int_payload)*index);
		}else{
			info.units[(int)u -1].sints15m[snum] = NULL;
		}
		info.units[(int)u -1].sints15m_cnt[snum] = index;
		break;
	case INT1D:
		if( index ){
			info.units[(int)u -1].sints1d[snum] = new endp_int_payload[index]; 
			memcpy(info.units[(int)u -1].sints1d[snum],ints,sizeof(endp_int_payload)*index);
		}else{
			info.units[(int)u -1].sints1d[snum] = NULL;
		}
		info.units[(int)u -1].sints1d_cnt[snum] = index;
		break;
	}

exit:
	if (req)
		delete req;
	if (resp)
		delete resp;
	return ret;
}

int 
accum_side(app_comm_cli &cli,channel_info_t &info,unit u,side s,output_t omode)
{
    endp_cur_payload *p,*p1;
    app_frame *req = NULL, *resp = NULL;
	int ret = 0, snum;
	unit_info_t &uinfo = info.units[(int)u - 1];

	if( !unit_is_ok(u,info) ){
		print_error(omode,"fatal error (%s): bad unit number: %d > %d",info.name,u,info.unit_cnt);
		exit(0);
	}

	if( setup_sides(u,info) ){
		print_error(omode,"fatal error (%s): bad unit number: %d",info.name,u);
		exit(0);
	}
	snum = side2index(s,u,info);
	
	// Get info about endpoint, loop 0
    req = new app_frame(APP_ENDP_CUR,APP_GET,app_frame::REQUEST,1,info.name);
	if( !req ){
		print_error(omode,"fatal error (%s): low on memory\n",info.name);
		exit(0);
	}
    p = (endp_cur_payload*)req->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = 0;

    if( ret = eocd_request(cli,req,resp,omode) ){ // no such unit or no net_side
    	print_errcode(omode,ret,info.name);
		goto exit;
    }

	p1 = (endp_cur_payload*)resp->payload_ptr();
	if( p1->unit != p->unit || p1->side != p->side || p1->loop != p->loop ){
		print_error(omode,"fatal error (%s): get information about wrong unit:\n"
			"\trequested: %s.%s.%s.%d\n"
			"\tgetted:    %s.%s.%s.%d\n",
			info.name,
			info.name,unit2string(u),side2string(s),p->loop,
			info.name,unit2string((unit)p1->unit),side2string((side)p1->side),p1->loop);
		exit(0);
	}
	
	if( snum < 0 ){
		print_error(omode,"fatal error (%s): wrong side: %d\n",info.name,s);
		exit(0);
	}
	uinfo.sides[snum] = *p1;
	uinfo.sints15m_cnt[snum] = 0;
	uinfo.sints1d_cnt[snum] = 0;
	if( info.tbl_type == TBL_FULL ){
		info.tbl_type = INT15;
		accum_ints(cli,info,u,s,omode);
		info.tbl_type = INT1D;		
		accum_ints(cli,info,u,s,omode);
		info.tbl_type = TBL_FULL;
	}else{
		accum_ints(cli,info,u,s,omode);
	}
exit:	
	if( req )
		delete req;
	if( resp )
		delete resp;
	return ret;
}

int
accum_unit(app_comm_cli &cli,channel_info_t &info,unit u,output_t omode)
{
	// Accumulate info about sides
	switch(u){
	case stu_c:
		if( accum_side(cli,info,u,cust_side,omode) ){
			print_error(omode,"fatal error(%s): get statistic from %s.%s.%s.0\n",
				info.name,info.name,unit2string(u),side2string(cust_side));
			exit(0);
		}
		break;
	case stu_r:
		if( accum_side(cli,info,u,net_side,omode) ){
			print_error(omode,"fatal error(%s): get statistic from %s.%s.%s.0\n",
				info.name,info.name,unit2string(u),side2string(net_side));
			exit(0);
		}
		break;
	default:
		if( accum_side(cli,info,u,cust_side,omode) ){
			print_error(omode,"fatal error(%s): get statistic from %s.%s.%s.0\n",
				info.name,info.name,unit2string(u),side2string(cust_side));
			exit(0);
		}

		if( accum_side(cli,info,u,net_side,omode) ){
			print_error(omode,"fatal error(%s): get statistic from %s.%s.%s.0\n",
				info.name,info.name,unit2string(u),side2string(net_side));
			exit(0);
		}
		break;
	}
	
	switch( u ){
	case stu_c:
	case stu_r:
		return 0;
	default:
		break;
	}
		// Get info about sensors
	app_frame *req = new app_frame(APP_SENSORS, APP_GET, app_frame::REQUEST, 1,
		info.name);
	app_frame *resp = NULL;
	char *buf;
	int ret = 0;

	if(!req){
		print_error(omode,"fatal error (%s): low on memory\n",info.name);
		exit(0);
	}

	((sensors_payload *)req->payload_ptr())->unit = u;

	if(ret = eocd_request(cli, req, resp, omode)){ // no such unit or no net_side
		print_errcode(omode,ret,info.name);
		goto exit;
	}else{
		sensors_payload *p = (sensors_payload *)resp->payload_ptr();
		unit_info_t &uinfo = info.units[(int)u-1];
		uinfo.sensors = *p;
	}
exit:
	if( resp )
	    delete resp;
	if( req )
	    delete req;
    return ret;
	
}

int
accum_channel(app_comm_cli &cli,channel_info_t &info,output_t omode,unit u)
{
    app_frame *req = NULL;
    app_frame *resp = NULL;
    char *buf;
	int size, ret = 0;

	if( info.type == slave ){
		return 0;
	}
	
	// Get channel parameters
	req = new app_frame(APP_SPAN_PARAMS,APP_GET,app_frame::REQUEST,1,info.name);
	if( !req ){
		print_error(omode,"fatal error (%s): low on memory\n",info.name);
		exit(0);
	}

    if( ret = eocd_request(cli,req,resp,omode) ){ // no such unit or no net_side
		print_errcode(omode,ret,info.name);
		goto exit;
    } else {
		span_params_payload *p = (span_params_payload *)resp->payload_ptr();
		info.unit_cnt = p->units;
		info.link_on = p->link_establ;
		if( p->loops != 1 ){
			print_error(omode,"error (%s): at this moment we don't support more than one loop",info.name);
			goto exit;
		}
		// Get information about units
		
		for(int i=0;i<p->units;i++)
			info.units_map[i] = false;
		switch( u ){
		case BCAST:
			for(int i=0;i<p->units;i++){
				info.units_map[i] = true;
				accum_unit(cli,info,(unit)(i+1),omode);
			}
			break;
		case unknown:
			break;
		default:
			info.units_map[(int)u-1] = true;
			accum_unit(cli,info,u,omode);
			break;
		}
	}
	if( req ){
		delete req;
	}
	if( resp ){
		delete resp; 
	}
	req = resp = NULL;
	
	// Get channel configuration
	req = new app_frame(APP_SPAN_CONF, APP_GET, app_frame::REQUEST, 1,info.name);
	if( !req ){
		print_error(omode,"fatal error (%s): low on memory\n",info.name);
		exit(0);
	}
	if ( ret = eocd_request(cli,req,resp,omode) ) {
		print_errcode(omode,ret,info.name);
		goto exit;
	} else {
		info.conf = *(span_conf_payload*)resp->payload_ptr();
	}

	if( req ){
		delete req;
	}
	if( resp ){
		delete resp; 
	}
	req = resp = NULL;

	// Get channel status
	req = new app_frame(APP_SPAN_STATUS, APP_GET, app_frame::REQUEST,1,info.name);
	if( !req ){
		print_error(omode,"fatal error (%s): low on memory\n",info.name);
		exit(0);
	}
	if ( ret = eocd_request(cli,req,resp,omode) ) {
		print_errcode(omode,ret,info.name);
		goto exit;
	} else {
		info.stat = *(span_status_payload*)resp->payload_ptr();
	}
	
exit:
	if (req)
		delete req;
	if (resp)
		delete resp;
	return ret;
}


//-------------------- Actions ---------------//
int rst_relative(app_comm_cli &cli,char *chan,unit u,side s,output_t omode)
{
    app_frame *req = new app_frame(APP_LOOP_RCNTRST,APP_GET,app_frame::REQUEST,1,chan);
    app_frame *resp = NULL;
    loop_rcntrst_payload *p;
	char *buf;
	int ret = 0;

	if( !req ){
		print_error(omode,"fatal error (%s): low on memory\n",chan);
		exit(0);
	}

	p = (loop_rcntrst_payload *)req->payload_ptr();
	p->unit = (u8)u;
	p->side = (u8)s;
	p->loop = (u8)0;
	
	if( (ret=eocd_request(cli,req,resp,omode)) ){ 
		print_errcode(omode,ret,chan);
	}
exit:
	if( req )
		delete req;
	if( resp )
		delete resp;
	return ret;
}


//----------------- Profile processing ------------------------------//
int accum_confprofile(app_comm_cli &cli, confprof_info_t &info,output_t omode)
{
	char *b;
	int size;
	// Endpoint current
	cprof_payload *p;
	app_frame *req = NULL, *resp = NULL;
	int ret = 0;
	int offset = 0;
	req = new app_frame(APP_CPROF, APP_GET, app_frame::REQUEST, 1, "");
	if( !req ){
		print_error(omode,"fatal error (%s): low on memory\n",info.pname);
		exit(0);
	}

	p = (cprof_payload*) req->payload_ptr();
	strncpy(p->pname, info.pname, SNMP_ADMIN_LEN);
	
	if( (ret=eocd_request(cli,req,resp,omode)) ){ 
		print_errcode(omode,ret," ");
		exit(0);
	}
	
	p = (cprof_payload*)resp->payload_ptr();
	info.conf = p->conf;
	info.comp = p->comp;
	if( req )
		delete req;
	if( resp )
		delete resp;
	return 0;
}

int accum_profiles(app_comm_cli &cli, profiles_info_t &info,output_t omode,char *pname)
{
	app_frame *req = NULL, *resp = NULL;
	cprof_list_payload *p = NULL, *p1 = NULL;
	int ret =0;
	int flag = 1;

	// Prepare info
	if( pname ){
		info.size = 1;
		info.cinfos = (confprof_info_t*)malloc(sizeof(confprof_info_t)*info.size);
		info.used = 0;
		confprof_info_t &pinfo = info.cinfos[info.used++];
		strncpy(pinfo.pname,pname,SNMP_ADMIN_LEN);
		accum_confprofile(cli,pinfo,omode);
		return 0;
	}
	info.size = 2;
	info.cinfos = (confprof_info_t*)malloc(sizeof(confprof_info_t)*info.size);
	info.used = 0;
	
	
	req = new app_frame(APP_LIST_CPROF, APP_GET, app_frame::REQUEST, 1, "");
	if( !req ){
		print_error(omode,"fatal error: low on memory\n");
		exit(0);
	}
	p = (cprof_list_payload*) req->payload_ptr();
	p->pname[0][0] = '\0';

	do {
		p1 = (cprof_list_payload*) req->payload_ptr();

		if( (ret=eocd_request(cli,req,resp,omode)) ){ 
			print_errcode(omode,ret,"profiles");
			exit(0);
		}

		p = (cprof_list_payload*) resp->payload_ptr();
		flag = !p->last_msg;
		for (int i = 0; i < p->filled; i++) {
			if( info.used >= info.size ){
				info.size *= 2;
				info.cinfos = (confprof_info_t*)realloc(info.cinfos,sizeof(confprof_info_t)*info.size);
				if( !info.cinfos ){
					print_error(omode,"fatal error: low on memory\n");
					exit(0);
				}
			}
			confprof_info_t &pinfo = info.cinfos[info.used++];
			strncpy(pinfo.pname,p->pname[i],SNMP_ADMIN_LEN);
			accum_confprofile(cli,pinfo,omode);
		}

		if (!p->filled)
			break;
		p1 = (cprof_list_payload*) req->payload_ptr();
		strncpy(p1->pname[0], p->pname[p->filled - 1], SNMP_ADMIN_LEN);
		delete resp;
		resp = NULL;
	} while(flag);

	if (req)
		delete req;
	if (resp)
		delete resp;
	return ret;
}

