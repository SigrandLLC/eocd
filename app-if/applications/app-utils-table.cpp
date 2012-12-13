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
#include "app-utils-table.h"

int max_es = sizeof("ES")-1, max_ses = sizeof("SES")-1, max_crc = sizeof("CRC")
	-1, max_uas = sizeof("UAS")-1, max_losws = sizeof("LOSWS")-1;
char es_fmt[32], ses_fmt[32], crc_fmt[32], losws_fmt[32], uas_fmt[32];
char es1_fmt[32], ses1_fmt[32], crc1_fmt[32], losws1_fmt[32], uas1_fmt[32];

int max_prof = sizeof("prof");
char prof_fmt[32];

void table_error_fmt(char *str)
{
	printf("\n****************************\n"
		"%s\n"
		"****************************\n", str);
}
/*
 void table_error_fmt(int ret,char *chan)
 {
 printf("\n\n****************************\n"
 "Error(%d): channel=%s. error=%s\n"
 "****************************\n\n"
 ,ret,chan,err_strings[ret - 1]);
 }
 */
int table_signs(u32 counter)
{
	int cnt;
	for(cnt = 0;counter;cnt++){
		counter /= 10;
	}
	return cnt;
}

int table_chars(char *s)
{
	int cnt;
	for(cnt = 0;*s;cnt++, s++)
		;
	return cnt;
}

void table_adjust_side(counters_t &cntrs)
{
	int tmp;
	if((tmp = table_signs(cntrs.es))>max_es)
		max_es = tmp;
	if((tmp = table_signs(cntrs.ses))>max_ses)
		max_ses = tmp;
	if((tmp = table_signs(cntrs.crc))>max_crc)
		max_crc = tmp;
	if((tmp = table_signs(cntrs.losws))>max_losws)
		max_losws = tmp;
	if((tmp = table_signs(cntrs.uas))>max_uas)
		max_uas = tmp;
}

void table_adjust(channel_info_t infos[], int info_num)
{
	for(int j = 0;j<info_num;j++){
		channel_info_t&info = infos[j];
		for(int i = 0;i<info.unit_cnt;i++){
			if(!info.units_map[i])
				continue;
			unit_info_t &uinfo = info.units[i];
			if(uinfo.have_cside){
				switch(info.tbl_type){
				case RELA:
					table_adjust_side(uinfo.sides[0].relative);
				case BASE:
					table_adjust_side(uinfo.sides[0].total);
					break;
				case INT15:
					table_adjust_side(uinfo.sides[0].cur15min);
					for(int k = 0;k<uinfo.sints15m_cnt[0];k++){
						table_adjust_side(uinfo.sints15m[0][k].cntrs);
					}
					break;
				case INT1D:
					table_adjust_side(uinfo.sides[0].cur1day);
					for(int k = 0;k<uinfo.sints1d_cnt[0];k++){
						table_adjust_side(uinfo.sints1d[0][k].cntrs);
					}
					break;
				}
			}
			if(uinfo.have_nside){
				switch(info.tbl_type){
				case BASE:
					table_adjust_side(uinfo.sides[1].total);
					break;
				case RELA:
					table_adjust_side(uinfo.sides[1].total);
					table_adjust_side(uinfo.sides[1].relative);
					break;
				case INT15:
					table_adjust_side(uinfo.sides[1].cur15min);
					for(int k = 0;k<uinfo.sints15m_cnt[1];k++){
						table_adjust_side(uinfo.sints15m[1][k].cntrs);
					}
					break;
				case INT1D:
					table_adjust_side(uinfo.sides[1].cur1day);
					for(int k = 0;k<uinfo.sints1d_cnt[1];k++){
						table_adjust_side(uinfo.sints1d[1][k].cntrs);
					}
					break;
				}
			}
		}
	}
	snprintf(es_fmt, 32, "%%-%dd|", max_es);
	snprintf(ses_fmt, 32, "%%-%dd|", max_ses);
	snprintf(crc_fmt, 32, "%%-%dd|", max_crc);
	snprintf(losws_fmt, 32, "%%-%dd|", max_losws);
	snprintf(uas_fmt, 32, "%%-%dd|", max_uas);

	snprintf(es1_fmt, 32, "%%-%ds|", max_es);
	snprintf(ses1_fmt, 32, "%%-%ds|", max_ses);
	snprintf(crc1_fmt, 32, "%%-%ds|", max_crc);
	snprintf(losws1_fmt, 32, "%%-%ds|", max_losws);
	snprintf(uas1_fmt, 32, "%%-%ds|", max_uas);
}

void table_short_adjust(channel_info_t infos[], int info_num)
{
	for(int j = 0;j<info_num;j++){
		channel_info_t&info = infos[j];
		int tmp = table_chars(info.conf.conf_prof);
		if(max_prof<tmp)
			max_prof = tmp;
	}
	snprintf(prof_fmt, 32, "%%-%ds|", max_prof);
}

int table_count_width(channel_info_t &info)
{
	int width = 0;

	switch(info.tbl_type){
	case RELA:
		width = RELA_WIDTH;
	case BASE:
		width += TABLE_WIDTH+max_es+max_ses+max_crc+max_losws+max_uas+5;
		break;
	case SENS:
		width = CHAN_WIDTH+UNIT_WIDTH+SIDE_WIDTH+3*SENSVAL_WIDTH+3
			*SENSTAT_WIDTH;
		break;
	case INT15:
		width = CHAN_WIDTH+UNIT_WIDTH+SIDE_WIDTH+DATE_WIDTH+2*TIME_WIDTH
			+MONI_WIDTH+max_es+max_ses+max_crc+max_losws+max_uas+5;
		break;
	case INT1D:
		width = CHAN_WIDTH+UNIT_WIDTH+SIDE_WIDTH+DATE_WIDTH+MONI_WIDTH+max_es
			+max_ses+max_crc+max_losws+max_uas+5;
		break;
	case SHORT:
		width = CHAN_WIDTH+REP_WIDTH+LINK_WIDTH+MODE_WIDTH+(max_prof+1) +
			RATE_WIDTH + ANX_WIDTH + TCPAM_WIDTH; // +
	default:
		break;
	}
	return width;
}

void table_delim(channel_info_t &info)
{
	int i = 0;
	int width = table_count_width(info);

	for(i = 0;i<width;i++){
		printf("-");
	}
	printf("\n");
}

void table_unit_delim(channel_info_t &info)
{
	int i = 0;
	for(i = 0;i<CHAN_WIDTH-1;i++){
		printf(" ");
	}
	printf("|");
	int width = table_count_width(info);
	for(i = 0;i<width-CHAN_WIDTH;i++){
		printf("-");
	}
	printf("\n");
}

void table_side_delim(channel_info_t &info)
{
	int i = 0;
	for(i = 0;i<CHAN_WIDTH-1;i++){
		printf(" ");
	}
	printf("|");
	for(i = 0;i<UNIT_WIDTH-1;i++){
		printf(" ");
	}
	printf("|");

	int width = table_count_width(info);
	width -= CHAN_WIDTH+UNIT_WIDTH;

	for(i = 0;i<width;i++){
		printf("-");
	}
	printf("\n");
}

void table_print_shorthead(channel_info_t &info)
{
	table_delim(info);
	printf("%-6s|", "chan");
	printf("%-4s|", "mode");
	printf("%-3s|", "rep");
	printf("%-4s|", "link");
	printf(prof_fmt, "prof");
	printf("%-5s|", "rate");
	printf("%-3s|", "anx");
	printf("%-8s|", "tcpam");
	printf("\n");
}

void table_print_head(channel_info_t &info)
{
	table_delim(info);
	printf("%-6s|", "chan");
	printf("%-5s|", "unit");
	if(info.tbl_type!=SENS)
		printf("%-8s|", "side");
	switch(info.tbl_type){
	case RELA:
		printf("%-1s|", "r");
	case BASE:
		printf("%-3s|", "SNR");
		printf("%-3s|", "ATN");
		printf(es1_fmt, "ES");
		printf(ses1_fmt, "SES");
		printf(crc1_fmt, "CRC");
		printf(losws1_fmt, "LOSWS");
		printf(uas1_fmt, "UAS");
		printf("\n");
		break;
	case SENS:
		printf("%-5s|", "s1");
		printf("%-5s|", "s2");
		printf("%-5s|\n", "s3");
		break;
	case INT15:
	case INT1D:
		printf("%-8s|", "date");
		if(info.tbl_type==INT15){
			printf("%-5s|", "start");
			printf("%-5s|", "end");
		}
		printf(es1_fmt, "ES");
		printf(ses1_fmt, "SES");
		printf(crc1_fmt, "CRC");
		printf(losws1_fmt, "LOSWS");
		printf(uas1_fmt, "UAS");
		printf("%-3s|", "%");
		printf("\n");
		break;
	}
}

void table_print_title(channel_info_t &info, int &d_ch, unit u, int &d_unit,
	side s, int &d_side)
{
	// channel field
	printf("%-6s|", d_ch ? info.name : " ");
	d_ch = 0;

	printf("%-5s|", d_unit ? unit2string(u) : " ");
	d_unit = 0;

	if(info.tbl_type!=SENS){
		printf("%-8s|", d_side ? side2string(s) : " ");
		d_side = 0;
	}
}

void table_print_row(channel_info_t &info, int &d_ch, unit u, int &d_unit,
	side s)
{
	int d_side = 1, snum = side2index(s, u, info);

	if(snum<0){
		table_error("fatal error (%s): wrong side: %d\n",info.name,s);
		exit(0);
	}

	endp_cur_payload &pay = info.units[(int)u-1].sides[snum];
	sensors_payload &spay = info.units[(int)u-1].sensors;

	switch(info.tbl_type){
	case BASE:
	case RELA:
		table_print_title(info, d_ch, u, d_unit, s, d_side);
		if(info.tbl_type==RELA){
			printf("%-1s|", "c");
		}
		printf("%-3d|%-3d|", pay.cur_snr, pay.cur_attn);
		printf(es_fmt, pay.total.es);
		printf(ses_fmt, pay.total.ses);
		printf(crc_fmt, pay.total.crc);
		printf(losws_fmt, pay.total.losws);
		printf(uas_fmt, pay.total.uas);
		printf("\n");

		if(info.tbl_type==RELA){
			table_print_title(info, d_ch, u, d_unit, s, d_side);
			printf("%-1s|", "r");
			printf("%-3s|%-3s|", "-", "-");
			printf(es_fmt, pay.relative.es);
			printf(ses_fmt, pay.relative.ses);
			printf(crc_fmt, pay.relative.crc);
			printf(losws_fmt, pay.relative.losws);
			printf(uas_fmt, pay.relative.uas);
			printf("\n");
		}
		break;
	case SENS:
		table_print_title(info, d_ch, u, d_unit, s, d_side);
		printf("%-1d|%-3d|", spay.state.sensor1, spay.sens1);
		printf("%-1d|%-3d|", spay.state.sensor2, spay.sens2);
		printf("%-1d|%-3d|\n", spay.state.sensor3, spay.sens2);
		break;
	case INT15:
		// Curren 15-minute interval
	{
		table_print_title(info, d_ch, u, d_unit, s, d_side);
		// Human readable time
		char s[256];
		time_t tm = time(NULL);
		tm -= tm%(15*60);
		strftime(s, 256, "%d.%m.%y", localtime(&tm));
		printf("%-8s|", s);

		strftime(s, 256, "%R", localtime(&tm));
		printf("%s|", s);

		printf("%-5s|", "NOW");

		printf(es_fmt, pay.cur15min.es);
		printf(ses_fmt, pay.cur15min.ses);
		printf(crc_fmt, pay.cur15min.crc);
		printf(losws_fmt, pay.cur15min.losws);
		printf(uas_fmt, pay.cur15min.uas);
		float percent = ((float)pay.cur15min.mon_sec/(15*60))*100; // Percentage of day
		printf("%-3d|", (int)percent);
		printf("\n");
	}
		// Previous 15-minute intervals
		for(int i = 0;i<info.units[(int)u-1].sints15m_cnt[snum];i++){
			table_print_title(info, d_ch, u, d_unit, s, d_side);
			endp_int_payload &ipay = info.units[(int)u-1].sints15m[snum][i];
			// Human readable time
			char s[256];
			time_t tm = time(NULL);
			int int_offs = ipay.int_num;
			if(tm%(15*60))
				int_offs--;
			tm -= tm%(15*60);
			tm -= 15*60*int_offs;
			strftime(s, 256, "%d.%m.%y", localtime(&tm));
			printf("%-8s|", s);

			tm -= 15*60;
			strftime(s, 256, "%R", localtime(&tm));
			printf("%s|", s);

			tm += 15*60;
			strftime(s, 256, "%R", localtime(&tm));
			printf("%s|", s);

			printf(es_fmt, ipay.cntrs.es);
			printf(ses_fmt, ipay.cntrs.ses);
			printf(crc_fmt, ipay.cntrs.crc);
			printf(losws_fmt, ipay.cntrs.losws);
			printf(uas_fmt, ipay.cntrs.uas);
			float percent = ((float)ipay.cntrs.mon_sec/(15*60))*100; // Percentage of day
			printf("%-3d|", (int)percent);
			printf("\n");
		}
		break;
	case INT1D:
		// Curren 1-day interval
	{
		table_print_title(info, d_ch, u, d_unit, s, d_side);
		// Human readable time
		char s[256];
		time_t tm = time(NULL);
		strftime(s, 256, "%d.%m.%y", localtime(&tm));
		printf("%-8s|", s);

		printf(es_fmt, pay.cur1day.es);
		printf(ses_fmt, pay.cur1day.ses);
		printf(crc_fmt, pay.cur1day.crc);
		printf(losws_fmt, pay.cur1day.losws);
		printf(uas_fmt, pay.cur1day.uas);
		float percent = ((float)pay.cur1day.mon_sec/(24*60*60))*100; // Percentage of day
		printf("%-3d|", (int)percent);
		printf("\n");
	}
		// Previous 1-day intervals

		for(int i = 0;i<info.units[(int)u-1].sints1d_cnt[snum];i++){
			table_print_title(info, d_ch, u, d_unit, s, d_side);
			endp_int_payload &ipay = info.units[(int)u-1].sints1d[snum][i];
			// Human readable time
			char s[256];
			time_t tm = time(NULL);
			int int_offs = ipay.int_num;
			/*if (tm % (24 * 60 * 60))
			 int_offs--;
			 tm -= tm % (24 * 60 * 60) + int_offs * 24 * 60 * 60
			 */
			tm -= int_offs*24*60*60;

			strftime(s, 256, "%d.%m.%y", localtime(&tm));
			printf("%-8s|", s);

			printf(es_fmt, ipay.cntrs.es);
			printf(ses_fmt, ipay.cntrs.ses);
			printf(crc_fmt, ipay.cntrs.crc);
			printf(losws_fmt, ipay.cntrs.losws);
			printf(uas_fmt, ipay.cntrs.uas);
			float percent = ((float)ipay.cntrs.mon_sec/(24*60*60))*100; // Percentage of day
			printf("%-3d|", (int)percent);
			printf("\n");
		}
		break;
	}
}

void table_side(channel_info_t &info, unit u, side s)
{
	int i = (int)u-1;
	int snum = side2index(s, u, info);
	int d_ch = 1;
	int d_unit = 1;

	if(snum<0){
		table_error("fatal error(%s): wrong parameters unit=%s, side=%s",
			info.name,unit2string(u),side2string(s));
		exit(0);
	}

	// Adjust column widths
	table_adjust(&info, 1);
	// Print head of table
	table_print_head(info);
	// Print delimiter
	table_delim(info);
	// Print information about all units
	switch(info.tbl_type){
	case SENS:
		// Side has no sensors. Only unit has.
	table_error("You cannot request sensor information from side.\nOnly Units equipped with sensors")
		;
		exit(0);
	default:
		break;
	}
	switch(s){
	case cust_side:
		if(info.units[i].have_cside){
			table_print_row(info, d_ch, u, d_unit, cust_side);
		}else{
			table_error("%s: %s has no %s",info.name,unit2string(u),side2string(s));
		}
		break;
	case net_side:
		if(info.units[i].have_nside){
			table_print_row(info, d_ch, u, d_unit, net_side);
		}else{
			table_error("%s: %s has no %s",info.name,unit2string(u),side2string(s));
		}
		break;
	}
}

void table_unit(channel_info_t &info, unit u)
{
	int d_ch = 1;
	int d_unit = 1;
	int i = (int)u-1;

	if(!unit_is_ok(u, info)){
		table_error("fatal error(%s) in table_unit: No such unit %d",info.name,u);
		exit(0);
	}
	// Adjust column widths
	table_adjust(&info, 1);
	// Print head of table
	table_print_head(info);
	// Print delimiter
	table_delim(info);
	// Print information about all units
	switch(info.tbl_type){
	case SENS:
		switch(u){
		case stu_c:
		case stu_r:
			// Terminal devices have no sensors
		table_error("Terminal devices have no sensors\n")
			;
			break;
		default:
			table_print_row(info, d_ch, u, d_unit, cust_side);
		}
		break;
	default:
		if(info.units[i].have_cside){
			table_print_row(info, d_ch, u, d_unit, cust_side);
		}
		if(info.units[i].have_nside){
			table_print_row(info, d_ch, u, d_unit, net_side);
		}
		break;
	}
	table_delim(info);
}

void table_channels(channel_info_t info[], int cnt)
{
	int d_ch = 1;

	if( cnt == 1 && info[0].type != master){
		table_error("Channel %s not maintain SHDSL data base",info[0].name);
		exit(0);
	}
	// Adjust column widths
	table_adjust(info, cnt);
	// Print head of table
	table_print_head(info[0]);
	// Print delimiter
	table_delim(info[0]);
	// Print information about all units
	for(int c = 0;c<cnt;c++){
		d_ch = 1;
		if( info[c].type != master )
			continue;
		for(int i = 0;i<info[c].unit_cnt;i++){
			int d_unit = 1;
			if(!info[c].units_map[i])
				continue;
			switch(info[0].tbl_type){
			case SENS:
				switch((unit)(i+1)){
				case stu_c:
				case stu_r:
					// Terminal devices have no sensors
					break;
				default:
					table_print_row(info[c], d_ch, (unit)(i+1), d_unit,
						cust_side);
				}
				break;
			default:
				if(info[c].units[i].have_cside){
					table_print_row(info[c], d_ch, (unit)(i+1), d_unit,
						cust_side);
				}
				if(info[c].units[i].have_nside){
					table_print_row(info[c], d_ch, (unit)(i+1), d_unit,
						net_side);
				}
				if(i!=info[c].unit_cnt-1)
					table_unit_delim(info[0]);
				break;
			}
		}
		table_delim(info[0]);
	}
}

void table_print_short(channel_info_t info[], int cnt)
{
	table_short_adjust(info, cnt);
	table_print_shorthead(info[0]);
	table_delim(info[0]);

	for(int i = 0;i<cnt;i++){
		printf("%-6s|", info[i].name);
		printf("%-4s|", (info[i].type==master) ? "mast" : "slav");

		if(info[i].type==master&&info[i].link_on)
			printf("%-3d|", info[i].stat.nreps);
		else
			printf("%-3s|", "-");

		if(info[i].type==master)
			printf("%-4s|", info[i].link_on ? "UP" : "DOWN");
		else
			printf("%-4s|", "-");

		printf(prof_fmt,(info[i].type==master) ? info[i].conf.conf_prof : "-");
		if(info[i].type==master)
			printf("%-5d|",info[i].stat.max_lrate);
		else
			printf("%-5s|","-");

		annex_t annex = annex_a;
		if( info[i].stat.region1 )
			annex = annex_b;
		printf("%-3s|",(info[i].type == master) ? annex2string(annex) : "-");
		printf("%-8s|",(info[i].type == master) ? tcpam2STRING((tcpam_t)info[i].stat.tcpam) : "-");
		printf("\n");
		table_delim(info[0]);
	}
}

//------------------ Profiles -----------------------------//

int table_count_width(profiles_info_t &info)
{
	int width = (max_prof+1) + ANX_WIDTH + RATE_WIDTH + PWR_WIDTH + TCPAM_WIDTH + COMPAT_WIDTH;
	return width;
}

void table_delim(profiles_info_t &info)
{
	int i = 0;
	int width = table_count_width(info);

	for(i = 0;i<width;i++){
		printf("-");
	}
	printf("\n");
}


void table_confprof_adjust(profiles_info_t &info)
{
	for(int i=0;i<info.used;i++){
		confprof_info_t &pinfo = info.cinfos[i];
		int tmp = table_chars(pinfo.pname);
		if(max_prof<tmp)
			max_prof = tmp;
	}
	snprintf(prof_fmt, 32, "%%-%ds|", max_prof);
}

void table_confprof_head(profiles_info_t &info)
{
	table_delim(info);
	printf(prof_fmt,"prof");
	printf("%-3s|","anx");
	printf("%-3s|","pwr");
	printf("%-5s|","rate");
	printf("%-8s|","tcpam");
	printf("%-8s|","compat");
	printf("\n");
	table_delim(info);
}

void table_confprof_row(confprof_info_t &info)
{
	printf(prof_fmt,info.pname);
	printf("%-3s|",annex2string(info.conf.annex));
	printf("%-3s|",power2string(info.conf.power));
	printf("%-5d|",info.conf.rate);
	printf("%-8s|",tcpam2string(info.conf.tcpam));
	char buf[256];
	EOC_dev::comp_name(info.comp, buf);
	printf("%-8s|",buf);
	printf("\n");
}

int
table_cprofiles(profiles_info_t &info)
{

	table_confprof_adjust(info);
	table_confprof_head(info);
	for(int i=0;i<info.used;i++){
		table_confprof_row(info.cinfos[i]);
		table_delim(info);
	}
	return 0;
}
