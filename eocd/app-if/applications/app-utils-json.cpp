extern "C" {
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
#include "app-utils-json.h"

int fd[2] = { -1, -1 };
FILE *out, *in;

#define bprintf(fmt,args...) {\
	fprintf(out,fmt,## args); \
}

//------------- JASON error handling. Print directly on stdout -------------//
void do_indent_dir(int indent) {
	for (int i = 0; i < indent; i++) {
		printf("    ");
	}
}

//
void json_error_fmt(int ret, int indent) {
	do_indent_dir(indent);
	printf("{\n");
	do_indent_dir(indent + 1);
	printf("\"eoc_error\": \"1\",\n");
	do_indent_dir(indent + 1);
	printf("\"err_string\" : \"(%d) %s\"\n", ret, err_strings[ret - 1]);
	do_indent_dir(indent);
	printf("}");
}

void json_error_fmt(char *s, int indent) {
	do_indent_dir(indent);
	printf("{\n");
	do_indent_dir(indent + 1);
	printf("\"eoc_error\": \"1\",\n");
	do_indent_dir(indent + 1);
	printf("\"err_string\" : \"%s\"\n", s);
	do_indent_dir(indent);
	printf("}\n");
}

//------------------ JASON payload. Print into temporary buffer.
// In error case ignore it, on success - print on stdout-----//


//---------- Temproary pipe - output temporary storage --------------//
int fd_set_nonblock(int fd, int nonblock) {
	int flags;
	char err_str[256];

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) {
		json_error("I/O error: fcntl(%d, F_GETFL) failed", fd);
		return -1;
	}

	if (nonblock)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0) {
		json_error("I/O error: fcntl(%d, F_SETFL) failed", fd);
		return -1;
	}
	return 0;
}

int json_init() {
	if (pipe(fd)) {
		return -1;
	}
	if (fd_set_nonblock(fd[0], 1))
		return -1;
	in = fdopen(fd[0], "r");
	out = fdopen(fd[1], "w");
	return 0;
}

int json_flush() {
	int buf[512];
	int count;
	int i = 0;

	fclose(out);

	while ((count = fread(buf, 1, 510, in)) > 0) {
		write(1, buf, count);
	}
}

void do_indent(int indent) {
	for (int i = 0; i < indent; i++) {
		bprintf("    ");
	}
}

// ------------- Information representation functions ------------------------//

void json_short_channel(int indent, struct eoc_channel *chan) {
	do_indent(indent);
	bprintf("{\n");
	do_indent(indent + 1);
	bprintf("\"name\" : \"%s\",\n",chan->name);
	do_indent(indent + 1);
	bprintf("\"type\" : \"%s\"\n",chan->t == slave ? "slave" : "master");
	do_indent(indent + 1);
	char buf[128];
	EOC_dev::comp_name(chan->comp,buf);
	bprintf("\"compat\" : \"%s\"\n",buf);
	do_indent(indent);
	bprintf("}");
}

int json_print_short(struct eoc_channel *channels, int cnum) {
	int indent = 0;
	int ret = 0;

	do_indent(indent);
	bprintf("{\n");

	do_indent(indent + 1);
	bprintf("\"channels\" : [\n");
	for (int i = 0; i < cnum; i++) {
		json_short_channel(indent + 2, channels + i);
		if (i < cnum - 1) {
			do_indent(indent);
			bprintf(",\n");
		}
		free(channels[i].name);
	}
	bprintf("\n");
	do_indent(indent + 1);
	bprintf("]\n");
	do_indent(indent);
	bprintf("}\n");

	return ret;
}

void json_sensor_events(int indent,int snum,unit_info_t &uinfo)
{
	struct sens_event *events = uinfo.sens_events[snum-1];
	int offset = 0;
	
	for(int i=0;i<uinfo.sens_events_num[snum-1];i++){
		do_indent(indent+offset);
		bprintf("{\n");
		offset++;
		do_indent(indent+offset);
		bprintf("\"event\": \"%d\",\n",events[i].index);

		char s[256];
		strftime(s, 256, "%d %b %G", localtime(&events[i].start));
		do_indent(indent + offset);
		bprintf("\"day_start\" : \"%s\",\n",s);

		strftime(s, 256, "%R", localtime(&events[i].start));
		do_indent(indent + offset);
		bprintf("\"time_start\" : \"%s\",\n",s);

		if( !events[i].end ){
			events[i].end = time(NULL);
		}
		strftime(s, 256, "%R", localtime(&events[i].end));
		do_indent(indent + offset);
		bprintf("\"time_end\" : \"%s\",\n",s);
		
		do_indent(indent + offset);
		bprintf("\"count\" : \"%d\"\n",events[i].cnt);
		
		offset--;
		do_indent(indent+offset);
		bprintf("}");
		if( i != uinfo.sens_events_num[snum-1]-1)
			bprintf(",");
		bprintf("\n");
	}
}

void json_sensor(int indent, int snum, int cur, int cnt, unit_info_t &uinfo) {
	do_indent(indent);
	bprintf("{\n");
	do_indent(indent + 1);
	bprintf("\"num\" : \"%d\",\n",snum);
	do_indent(indent + 1);
	bprintf("\"cur\" : \"%d\",\n",cur);
	do_indent(indent + 1);
	bprintf("\"cnt\" : \"%d\"\n",cnt);
	// Sensor events
	do_indent(indent+1);
	bprintf("\"events\" : [\n",cnt);
	json_sensor_events(indent+2,snum,uinfo);
	do_indent(indent+1);
	bprintf("]\n");
	
	do_indent(indent);
	bprintf("}");
}


int 
json_sensors(int indent, channel_info_t &info, unit u) 
{
	if( !unit_is_ok(u,info) ){
		json_error("Fatal error. Channel %s has no unit %s",info.name,unit2string(u));
		return -1;
	}
	sensors_payload &p = info.units[(int)u-1].sensors;
	json_sensor(indent, 1, p.state.sensor1, p.sens1,info.units[(int)u-1]);
	bprintf(",\n");
	json_sensor(indent, 2, p.state.sensor2, p.sens2,info.units[(int)u-1]);
	bprintf(",\n");
	json_sensor(indent, 3, p.state.sensor3, p.sens3,info.units[(int)u-1]);
	bprintf("\n");
	return 0;
}



int json_spanconf(int indent, channel_info_t &info)
{
	span_conf_payload &p = info.conf;
	do_indent(indent);
	bprintf("\"conf\" : {\n");
	if (p.type == master) {
		do_indent(indent + 1);
		bprintf("\"adm_reg_num\" : \"%d\",\n",p.nreps);
		do_indent(indent + 1);
		bprintf("\"naprof\" : \"%s\",\n",p.alarm_prof);
	}
	do_indent(indent + 1);
	bprintf("\"type\" : \"%s\",\n",(p.type==master) ? "master" : "slave");
	do_indent(indent + 1);
	bprintf("\"cprof\" : \"%s\"\n",p.conf_prof);
	do_indent(indent);
	bprintf("}");
	return 0;
}

int json_spanstat(int indent, channel_info_t &info)
{
	span_status_payload &p = info.stat;
	annex_t annex = annex_a;
	if (p.region1)
		annex = annex_b;
	do_indent(indent);
	bprintf("\"status\" : {\n");
	do_indent(indent + 1);
	bprintf("\"reg_num\" : \"%d\",\n",p.nreps);
	do_indent(indent + 1);
	bprintf("\"rate\" : \"%d\",\n",p.max_lrate);
	do_indent(indent + 1);
	bprintf("\"annex\" : \"%s\",\n",annex2string(annex));
	do_indent(indent + 1);
	bprintf("\"tcpam\" : \"%s\"\n",tcpam2STRING((tcpam_t)p.tcpam));
	do_indent(indent);
	bprintf("}");
}

int json_channel(int indent, channel_info_t &info)
{
	if( info.type != master ){
		json_error("Channel %s not maintain SHDSL base",info.name);
		exit(0);
	}
	do_indent(indent);
	bprintf("{\n");
	do_indent(indent + 1);
	bprintf("\"interface\" : \"%s\",\n",info.name);
	do_indent(indent + 1);
	bprintf("\"unit_num\" : \"%d\",\n",info.unit_cnt);
	do_indent(indent + 1);
	bprintf("\"link\" : \"%d\",\n",info.link_on);
	do_indent(indent + 1);
	bprintf("\"loop_num\" : \"%d\",\n",1); // At this moment only one loop supported
	json_spanconf(indent + 1, info);
	bprintf(",\n");
	json_spanstat(indent + 1, info);
	bprintf("\n");
	do_indent(indent);
	bprintf("}");
	return 0;
}


int
json_m15ints(int indent, channel_info_t &info, unit u, side s)
{
	int offset = 0;
	int snum = side2index(s,u,info);
	if( snum < 0 ){
		json_error("Fatal error. Cannot resolv side=%d of unit=%d to index",s,u);
		return -1;
	}
	if( info.tbl_type != TBL_FULL ){
		json_error("Fatal error. Not all information accumulated for channel \'%s\'",info.name);
		exit(1);
	}
	
	endp_int_payload *ints = info.units[(int)u-1].sints15m[snum];
	int cnt = info.units[(int)u-1].sints15m_cnt[snum];

	for (int i = 0; i < cnt; i++) {
		do_indent(indent + offset);
		bprintf("{\n");
		offset++;

		do_indent(indent + offset);
		bprintf("\"int\" : \"%d\",\n",ints[i].int_num);

		// Human readable time
		char s[256];
		time_t tm = time(NULL);
		int int_offs = ints[i].int_num;
		if (tm % (15 * 60))
			int_offs--;
		tm -= tm % (15 * 60);
		tm -= 15 * 60 * int_offs;
		strftime(s, 256, "%d %b %G", localtime(&tm));
		do_indent(indent + offset);
		bprintf("\"int_day\" : \"%s\",\n",s);

		strftime(s, 256, "%R", localtime(&tm));
		do_indent(indent + offset);
		bprintf("\"time_end\" : \"%s\",\n",s);

		tm -= 15 * 60;
		strftime(s, 256, "%R", localtime(&tm));
		do_indent(indent + offset);
		bprintf("\"time_start\" : \"%s\",\n",s);

		// Statistics
		float percent = ((float) ints[i].cntrs.mon_sec / (15 * 60)) * 100; // Percentage of day

		do_indent(indent + offset);
		bprintf("\"es\" : \"%u\",\n",ints[i].cntrs.es);
		do_indent(indent + offset);
		bprintf("\"ses\" : \"%u\",\n",ints[i].cntrs.ses);
		do_indent(indent + offset);
		bprintf("\"crc\" : \"%d\",\n",ints[i].cntrs.crc);
		do_indent(indent + offset);
		bprintf("\"losws\" : \"%u\",\n",ints[i].cntrs.losws);
		do_indent(indent + offset);
		bprintf("\"uas\" : \"%u\",\n",ints[i].cntrs.uas);
		do_indent(indent + offset);
		bprintf("\"mon_sec\" : \"%d\",\n",ints[i].cntrs.mon_sec);
		do_indent(indent + offset);
		bprintf("\"mon_pers\" : \"%.2f\"\n",percent);

		offset--;
		do_indent(indent + offset);
		if( i == (cnt-1) ){
			bprintf("}\n");
		}else{
			bprintf("},\n");
		}
		
	}
	return 0;
}


int
json_d1ints(int indent, channel_info_t &info, unit u, side s)
{
	int offset = 0;
	int snum = side2index(s,u,info);
	if( snum < 0 ){
		json_error("Fatal error. Cannot resolv side=%d of unit=%d to index",s,u);
		exit(0);
	}
	if( info.tbl_type != TBL_FULL ){
		json_error("Fatal error. Not all information accumulated for channel %s",info.name);
		exit(1);
	}
	
	endp_int_payload *ints = info.units[(int)u-1].sints1d[snum];
	int cnt = info.units[(int)u-1].sints1d_cnt[snum];
	
	for (int i = 0; i < cnt; i++) {
		do_indent(indent + offset);
		bprintf("{\n");
		offset++;

		do_indent(indent + offset);
		bprintf("\"int\" : \"%d\",\n",ints[i].int_num);

		// Human readable time
		char s[256];
		time_t tm = time(NULL);
		int int_offs = ints[i].int_num;
		/*
		if (tm % (24 * 60 * 60))
			int_offs--;
		tm -= tm % (24 * 60 * 60) + int_offs * 24 * 60 * 60;
		*/
		tm -= int_offs * 24 * 60 * 60;
		strftime(s, 256, "%d %b %G", localtime(&tm));
		do_indent(indent + offset);
		bprintf("\"int_day\" : \"%s\",\n",s);

		// Statistics
		float percent = ((float) ints[i].cntrs.mon_sec / (24 * 60 * 60)) * 100; // Percentage of day

		do_indent(indent + offset);
		bprintf("\"es\" : \"%u\",\n",ints[i].cntrs.es);
		do_indent(indent + offset);
		bprintf("\"ses\" : \"%u\",\n",ints[i].cntrs.ses);
		do_indent(indent + offset);
		bprintf("\"crc\" : \"%d\",\n",ints[i].cntrs.crc);
		do_indent(indent + offset);
		bprintf("\"losws\" : \"%u\",\n",ints[i].cntrs.losws);
		do_indent(indent + offset);
		bprintf("\"uas\" : \"%u\",\n",ints[i].cntrs.uas);
		do_indent(indent + offset);
		bprintf("\"mon_sec\" : \"%d\",\n",ints[i].cntrs.mon_sec);
		do_indent(indent + offset);
		bprintf("\"mon_pers\" : \"%.2f\"\n",percent);

		offset--;
		do_indent(indent + offset);
		if( i == (cnt-1) ){
			bprintf("}\n");
		}else{
			bprintf("},\n");
		}
	}
	return 0;
}

int
json_side(int indent,channel_info_t &info, unit u, side s)
{
	int offset = 0;
	int ret = 0;
	// Print out full info about unit
	do_indent(indent);
	bprintf("{\n");
	offset++;;

	do_indent(indent+offset);
	bprintf("\"name\" : \"%s\",\n",side2string(s));
	do_indent(indent + offset);
	bprintf("\"loops\" : [\n");
	indent++;
	
	// At this moment we have only one loop
	int loop = 0;
	int snum = side2index(s,u,info);
	if( snum < 0 ){
		json_error("Fatal error. Cannot resolv side=%d of unit=%d to index",(int)s,(int)u);
		return -1;
	}
	
	endp_cur_payload &cpay = info.units[(int)u-1].sides[snum];

	do_indent(indent + offset);
	bprintf("{\n");
	offset++;

	do_indent(indent + offset);
	bprintf("\"name\" : \"Pair%d\",\n",loop);
	// Current counters
	do_indent(indent + offset);
	bprintf("\"cur\" : {\n");
	offset++;
	do_indent(indent + offset);
	bprintf("\"snr\" : \"%d\",\n",cpay.cur_snr);
	do_indent(indent + offset);
	bprintf("\"lattn\" : \"%d\",\n",cpay.cur_attn);
	do_indent(indent + offset);
	bprintf("\"es\" : \"%u\",\n",cpay.total.es);
	do_indent(indent + offset);
	bprintf("\"ses\" : \"%u\",\n",cpay.total.ses);
	do_indent(indent + offset);
	bprintf("\"crc\" : \"%d\",\n",cpay.total.crc);
	do_indent(indent + offset);
	bprintf("\"losws\" : \"%u\",\n",cpay.total.losws);
	do_indent(indent + offset);
	bprintf("\"uas\" : \"%u\",\n",cpay.total.uas);

	// current 15 minute interval counters
	do_indent(indent + offset);
	bprintf("\"m15int\" : {\n");
	offset++;

	do_indent(indent + offset);
	bprintf("\"es\" : \"%u\",\n",cpay.cur15min.es);
	do_indent(indent + offset);
	bprintf("\"ses\" : \"%u\",\n",cpay.cur15min.ses);
	do_indent(indent + offset);
	bprintf("\"crc\" : \"%d\",\n",cpay.cur15min.crc);
	do_indent(indent + offset);
	bprintf("\"losws\" : \"%u\",\n",cpay.cur15min.losws);
	do_indent(indent + offset);
	bprintf("\"uas\" : \"%u\",\n",cpay.cur15min.uas);
	do_indent(indent + offset);
	bprintf("\"mon_sec\" : \"%u\",\n",cpay.cur15min.mon_sec);
	do_indent(indent + offset);
	bprintf("\"elapsed\" : \"%02dm:%02ds\"\n",(cpay.cur_15m_elaps%(60*60))/60,cpay.cur_15m_elaps%60);
	offset--;
	do_indent(indent + offset);
	bprintf("},\n");

	// current 1 day interval counters
	do_indent(indent + offset);
	bprintf("\"d1int\" : {\n");
	offset++;

	do_indent(indent + offset);
	bprintf("\"es\" : \"%u\",\n",cpay.cur1day.es);
	do_indent(indent + offset);
	bprintf("\"ses\" : \"%u\",\n",cpay.cur1day.ses);
	do_indent(indent + offset);
	bprintf("\"crc\" : \"%d\",\n",cpay.cur1day.crc);
	do_indent(indent + offset);
	bprintf("\"losws\" : \"%u\",\n",cpay.cur1day.losws);
	do_indent(indent + offset);
	bprintf("\"uas\" : \"%u\",\n",cpay.cur1day.uas);
	do_indent(indent + offset);
	bprintf("\"mon_sec\" : \"%u\",\n",cpay.cur1day.mon_sec);
	do_indent(indent + offset);
	bprintf("\"elapsed\" : \"%02dh:%02dm:%02ds\"\n",
		cpay.cur_1d_elaps/(60*60),(cpay.cur_1d_elaps%(60*60))/60,cpay.cur_1d_elaps%60);

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
	char date[64];
	strftime(date, 64, "%d %b %G", localtime(&cpay.relative_ts));
	bprintf("\"date\" : \"%s\",\n",date);
	do_indent(indent + offset);
	strftime(date, 256, "%R", localtime(&cpay.relative_ts));
	bprintf("\"time\" : \"%s\",\n",date);
	do_indent(indent + offset);
	bprintf("\"es\" : \"%u\",\n",cpay.relative.es);
	do_indent(indent + offset);
	bprintf("\"ses\" : \"%u\",\n",cpay.relative.ses);
	do_indent(indent + offset);
	bprintf("\"crc\" : \"%d\",\n",cpay.relative.crc);
	do_indent(indent + offset);
	bprintf("\"losws\" : \"%u\",\n",cpay.relative.losws);
	do_indent(indent + offset);
	bprintf("\"uas\" : \"%u\"\n",cpay.relative.uas);
	offset--;
	do_indent(indent + offset);
	bprintf("},\n");

	// 15-min intervals
	do_indent(indent + offset);
	bprintf("\"m15int\" : [\n");
	json_m15ints(indent+offset+1,info,u,s);
	bprintf("\n");
	do_indent(indent + offset);
	bprintf("],\n");

	// 1-day intervals
	do_indent(indent + offset);
	bprintf("\"d1int\" : [\n");
	json_d1ints(indent+offset+1,info,u,s);
	bprintf("\n");
	do_indent(indent + offset);
	bprintf("]\n");

	offset--;
	do_indent(indent + offset);
	bprintf("}");
	bprintf("\n");

	offset--;
	do_indent(indent + offset);
	bprintf("]\n");

	offset--;
	do_indent(indent + offset);
	bprintf("}");

	return ret;
}

int
json_unit(int indent,channel_info_t &info, unit u)
{
	int ret = 0;
	int offset = 0;

	if( !unit_is_ok(u,info) ){
		// Unit bigger than aviliable
		json_error("Wrong unit: %d",u);
		exit(1);
	}
	
	do_indent(indent + offset);
	bprintf("{\n");
	offset++;
	do_indent(indent + offset);
	bprintf("\"unit\" : \"%s\",\n",unit2string(u));
	// Unit sensors info. Only SRUx has sensors
	switch (u) {
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
		json_sensors(indent + offset + 1,info, u);
		bprintf("\n");
		offset--;
		do_indent(indent + offset);
		bprintf("],\n");
		break;
	}

	do_indent(indent + offset);
	bprintf("\"sides\" : [\n",unit2string(u));
	offset++;

	switch (u) {
	case stu_c:
		ret = json_side(indent + offset,info,u,cust_side);
		break;
	case stu_r:
		ret = json_side(indent + offset,info,u, net_side);
		break;
	case sru1:
	case sru2:
	case sru3:
	case sru4:
	case sru5:
	case sru6:
	case sru7:
	case sru8:
		ret = json_side(indent + offset, info, u, cust_side);
		bprintf(",\n");
		ret += json_side(indent + offset, info, u, net_side);
		break;
	default: {
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
	json_error("Incorrect channel unit: %d", u);
	return -1;
}

void json_one_confprof(int indent,confprof_info_t &info)
{
	int offset = 0;
	do_indent(indent+offset);
	bprintf("{\n");
	offset++;
	do_indent(indent+offset);
	bprintf("\"name\" : \"%s\",\n",info.pname);
	do_indent(indent+offset);
	bprintf("\"annex\" : \"%s\",\n",annex2string(info.conf.annex));
	do_indent(indent+offset);
	bprintf("\"power\" : \"%s\",\n",power2string(info.conf.power));
	do_indent(indent+offset);
	bprintf("\"rate\" : \"%d\",\n",info.conf.rate);
	do_indent(indent+offset);
	bprintf("\"tcpam\" : \"%s\"\n",tcpam2string(info.conf.tcpam));
	char buf[256];
	EOC_dev::comp_name(info.comp, buf);
	do_indent(indent+offset);
	bprintf("\"comp\" : \"%s\"\n",buf);

	offset--;
	do_indent(indent+offset);
	bprintf("}");
}

int 
json_cprofiles(profiles_info_t &info)
{
	int ret = 0;
	int indent = 0, offset = 0;

	do_indent(indent + offset);
	bprintf("{\n");
	offset++;

	do_indent(indent + offset);
	bprintf("\"profiles\" : [\n");
	offset++;

	for(int i=0;i<info.used;i++){
		json_one_confprof(indent + offset,info.cinfos[i]);
		if ( i != (info.used-1) ) {
			bprintf(",\n");
		} else {
			bprintf("\n");
		}
	}

	offset--;
	do_indent(indent + offset);
	bprintf("]\n");

	offset--;
	do_indent(indent + offset);
	bprintf("}\n");
	return 0;
}
