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


bool
unit_is_ok(unit u,channel_info_t &info)
{
	int i = (int)u-1; 
	if( !(i < info.unit_cnt) || !info.units_map[i] )
		return false;
	return true;
}

int setup_sides(unit u,channel_info_t &info)
{
	switch( u ){
	case stu_c:
		info.units[(int)u-1].have_cside = 1;
		break;
	case stu_r:
		info.units[(int)u-1].have_nside = 1;
		break;
	default:
		if( u >= sru1 && u <= sru10 ){
			info.units[(int)u-1].have_nside = 1;
			info.units[(int)u-1].have_cside = 1;
		}else{
			return -1;
		}
		break;
	}
	return 0;
}

int
side2index(side s,unit u,channel_info_t &info)
{
	if( !unit_is_ok(u,info) )
		return -1;
	switch( s ){
	case cust_side:
		if( !info.units[(int)u-1].have_cside )
			return -1;
		return 0;
	case net_side:
		if( !info.units[(int)u-1].have_nside )
			return -1;
		return 1;
	}
	return -1;
}

void
init_chan_info(struct eoc_channel &ch,channel_info_t &info)
{
	table_type_t bkp = info.tbl_type;
	memset(&info,0,sizeof(info));
	info.tbl_type = bkp;
	strncpy(info.name,ch.name,32);
	info.type = ch.t;
	info.comp = ch.comp;
}


