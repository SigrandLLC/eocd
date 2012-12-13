#ifndef APP_UTILS_H
#define APP_UTILS_H

#include <generic/EOC_generic.h>
#include <db/EOC_unit.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>

typedef enum {NORMAL,JSON} output_t;
typedef enum {NONE,HELP,CHANNEL,PROFILE} object_t;
struct eoc_channel{
	char *name;
	dev_type t;
	EOC_dev::compatibility_t comp;
};

typedef enum {BASE, RELA, SENS,INT15,INT1D,TBL_FULL,SHORT,SENS_FULL } table_type_t;
typedef enum {noint, m15int,d1int } interval_t;

typedef struct{
	u8 have_cside : 1;
	u8 have_nside : 1;
	endp_cur_payload sides[2];
	sensors_payload sensors;
	struct sens_event sens_events[3][SENS_EVENTS_NUM];
	int sens_events_num[3];
	int sints15m_cnt[2];
	endp_int_payload *sints15m[2];
	int sints1d_cnt[2];
	endp_int_payload *sints1d[2];
} unit_info_t;

typedef struct {
	table_type_t tbl_type;
	char name[32];
	dev_type type;
	EOC_dev::compatibility_t comp;
	int unit_cnt;
	int link_on;
	bool units_map[10];
	unit_info_t units[10];
	span_conf_payload conf;
	span_status_payload stat;
} channel_info_t;

typedef struct {
    char pname[SNMP_ADMIN_LEN+1];
    span_conf_profile_t conf;
    EOC_dev::compatibility_t comp;
} confprof_info_t;

typedef struct {
	int size,used;
	confprof_info_t *cinfos;
} profiles_info_t;

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
bool unit_is_ok(unit u,channel_info_t &info);
int side2index(side s,unit u,channel_info_t &info);
int setup_sides(unit u,channel_info_t &info);
void init_chan_info(struct eoc_channel &ch,channel_info_t &info);

#endif
