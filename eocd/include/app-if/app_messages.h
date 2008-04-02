#ifndef APP_MESSAGES_H
#define APP_MESSAGES_H

#include <snmp/snmp-generic.h>
#include <generic/span_conf_type.h>
#include <generic/EOC_responses.h>
#include <generic/EOC_types.h>


typedef enum { APP_SPAN_NAME=0,APP_SPAN_PARAMS,APP_SPAN_CONF,APP_SPAN_STATUS,
			   APP_INVENTORY, APP_ENDP_CONF, APP_ENDP_CUR, APP_ENDP_15MIN,
			   APP_ENDP_1DAY, APP_ENDP_MAINT, APP_UNIT_MAINT, APP_CPROF,
			   APP_LIST_CPROF, APP_ADD_CPROF, APP_DEL_CPROF, APP_LOOP_RCNTRST,
			   APP_ADD_CHAN, APP_DEL_CHAN, APP_CHNG_CHAN, APP_ENDP_APROF,
			   APP_DUMP_CFG,APP_SENSORS, APP_PBO
} app_ids;

#define app_ids_num 20

typedef enum { APP_SET,APP_GET,APP_GET_NEXT } app_types;

/*
1. список обслуживаемых каналов
2. Для конкретного канала: число элементов в канале, число пар проводов
*/

#define SPAN_NAMES_NUM 4
#define SPAN_NAME_LEN 32


struct span_desc{
    char name[SPAN_NAME_LEN];
	dev_type t;
};

typedef struct{
    u8 filled:7;
    u8 last_msg:1; 
	struct span_desc spans[SPAN_NAMES_NUM];
} span_name_payload;
#define SPAN_NAME_PAY_SZ sizeof(span_name_payload)
#define SPAN_NAME_CH_SZ 0

typedef struct{
    u8 units;
    u8 link_establ:1;
    u8 loops:7;
} span_params_payload;
#define SPAN_PARAMS_PAY_SZ sizeof(span_params_payload)
#define SPAN_PARAMS_CH_SZ 0

typedef struct{
	dev_type type;
    s16 nreps;
    char conf_prof[SNMP_ADMIN_LEN+1];
    char alarm_prof[SNMP_ADMIN_LEN+1];
} span_conf_payload;
#define SPAN_CONF_PAY_SZ sizeof(span_conf_payload)
#define SPAN_CONF_CH_SZ 0

typedef struct{
    s8 nreps;
    u32 max_lrate;
    u32 act_lrate;
    u8 region0 : 1;
    u8 region1 : 1;
    u8 : 6;
    u32 max_prate;
    u32 act_prate;
	u8 tcpam;
} span_status_payload;
#define SPAN_STATUS_PAY_SZ sizeof(span_status_payload)
#define SPAN_STATUS_CH_SZ 0

typedef struct{
    u8 unit;
    u8 eoc_softw_ver;
    resp_inventory inv;
    u8 region0 :1;
    u8 region1 :1;
} inventory_payload;
#define INVENTORY_PAY_SZ sizeof(inventory_payload)
#define INVENTORY_CH_SZ 0

typedef struct{
    u8 unit;
    u8 side;
    u8 loop;
    char alarm_prof[SNMP_ADMIN_LEN+1];
} endp_conf_payload;
#define ENDP_CONF_PAY_SZ sizeof(endp_conf_payload)
// TODO: may be set request!
#define ENDP_CONF_CH_SZ 0

typedef struct {
    u8 unit;
    u8 side;
    u8 loop;
    s32 cur_attn;
    s32 cur_snr;
    shdsl_status_t cur_status;
    counters_t total;
    counters_t relative;
	time_t relative_ts;
    u32 cur_15m_elaps;
    counters_t cur15min;
    u32 cur_1d_elaps;    
    counters_t cur1day;
    s32 CurrTipRingReversal;
    s32 CurrActivationState;
} endp_cur_payload;
#define ENDP_CUR_PAY_SZ sizeof(endp_cur_payload)
#define ENDP_CUR_CH_SZ sizeof(endp_cur_payload)

typedef struct {
    u8 unit;
    u8 side;
    u8 loop;
    u32 int_num;
    counters_t cntrs;
} endp_int_payload;

#define endp_15min_payload endp_int_payload
#define ENDP_15MIN_PAY_SZ sizeof(endp_15min_payload)
#define ENDP_15MIN_CH_SZ 0

#define endp_1day_payload endp_int_payload
#define ENDP_1DAY_PAY_SZ sizeof(endp_15min_payload)
#define ENDP_1DAY_CH_SZ 0


typedef struct{
    u8 unit;
    u8 side;
    s32 LoopbackConfig;
    s32 TipRingReversal;
    s32 PowerBackOff;
    s32 SoftRestart;
} endp_maint_payload;
#define ENDP_MAINT_PAY_SZ sizeof(endp_maint_payload)

typedef struct{
    u8 LoopbackConfig:1;
    u8 PowerBackOff:1;
    u8 SoftRestart:1;
} endp_maint_changes;
#define ENDP_MAINT_CH_SZ sizeof(endp_maint_changes)

typedef struct{
    s32 LoopbackTimeout;
    s32 PowerSource;
} unit_maint_payload;
#define UNIT_MAINT_PAY_SZ sizeof(unit_maint_changes)

typedef struct{
    u8 LoopbackTimeout:1;
} unit_maint_changes;
#define UNIT_MAINT_CH_SZ sizeof(unit_maint_changes)

//------- Span Configuration -------------//

typedef struct{
    char pname[SNMP_ADMIN_LEN+1];
} cprof_add_payload;
#define CPROF_ADD_PAY_SZ sizeof(cprof_add_payload)
#define CPROF_ADD_CH_SZ 0


typedef struct{
    char pname[SNMP_ADMIN_LEN+1];
} cprof_del_payload;
#define CPROF_DEL_PAY_SZ sizeof(cprof_del_payload)
#define CPROF_DEL_CH_SZ 0


typedef struct{
    char pname[SNMP_ADMIN_LEN+1];
    span_conf_profile_t conf;
    u8 filled:7;
	char names[SPAN_NAMES_NUM][SPAN_NAME_LEN];
} cprof_payload;
#define CPROF_PAY_SZ sizeof(cprof_payload)

typedef struct{
    u8 annex :1;
    u8 wires :1;      
    u8 power :1;
    u8 psd :1;
    u8 clk:1;
    u8 line_probe:1;
    u8 remote_cfg:1;
    u8 rate:1;
    u8 tcpam:1;
    s8 cur_marg_down:1;
    s8 worst_marg_down:1;
    s8 cur_marg_up:1;
    s8 worst_marg_up:1;
    u8 use_cur_down :1;                                                   
    u8 use_worst_down :1;                                                  
    u8 use_cur_up :1;                                                     
    u8 use_worst_up :1;
} cprof_changes;
#define CPROF_CH_SZ sizeof(cprof_changes)


#define PROF_NAMES_NUM 8
typedef struct{
    u8 filled:7;
    u8 last_msg:1; 
    char pname[PROF_NAMES_NUM][SNMP_ADMIN_LEN+1];
} cprof_list_payload;
#define CPROF_LIST_PAY_SZ sizeof(cprof_list_payload)
#define CPROF_LIST_CH_SZ 0

//--------------- Endpoint Alarm -----------------//

typedef struct{
    char ProfileName[SNMP_ADMIN_LEN];
    s32 ThreshLoopAttenuation;
    s32 ThreshSNRMargin;
    u32 ThreshES;
    u32 ThreshSES;
    u32 ThreshCRCanomalies;
    u32 ThreshLOSWS;
    u32 hdsl2ShdslEndpointThreshUAS;
} endp_alarm_prof_payload;
#define ENDP_ALARM_PROF_PAY_SZ sizeof(endp_alarm_prof_payload)

typedef struct{
    u8 ThreshLoopAttenuation:1;
    u8 ThreshSNRMargin:1;
    u8 ThreshES:1;
    u8 ThreshSES:1;
    u8 ThreshCRCanomalies:1;
    u8 ThreshLOSWS:1;
    u8 hdsl2ShdslEndpointThreshUAS:1;
} endp_alarm_prof_changes;
#define ENDP_ALARM_PROF_CH_SZ sizeof(endp_alarm_prof_changes)

//----------- Regenerator Sensors ----------------//

typedef struct{
    u8 unit;
    resp_sensor_state state;
	u8 sens1,sens2,sens3;
} sensors_payload;
#define SENSORS_PAY_SZ sizeof(sensors_payload)
#define SENSORS_CH_SZ 0


//----------- Endpoint counters reset ------------//

typedef struct{
    u8 unit;
    u8 side;
	u8 loop;
} loop_rcntrst_payload;
#define LOOP_RCNTRST_PAY_SZ sizeof(loop_rcntrst_payload)
#define LOOP_RCNTRST_CH_SZ 0

//----------- Channel add/del/change -------------//

typedef struct{
	u8 master:1;
}chan_add_payload;
#define CHAN_ADD_PAY_SZ sizeof(chan_add_payload)
#define CHAN_ADD_CH_SZ 0

typedef struct{
}chan_del_payload;
#define CHAN_DEL_PAY_SZ sizeof(chan_del_payload)
#define CHAN_DEL_CH_SZ 0

typedef struct{
	char cprof[SNMP_ADMIN_LEN];
	u8 master:1;
	u8 master_ch:1;
	u8 rep_num:4;
	u8 rep_num_ch:1;
	u8 cprof_ch:1;
	u8 apply_conf:1;
	u8 apply_conf_ch:1;
} chan_chng_payload;
#define CHAN_CHNG_PAY_SZ sizeof(chan_chng_payload)
#define CHAN_CHNG_CH_SZ 0

typedef struct{
	u8 mode;
	char val[PBO_SETTING_LEN];
} chan_pbo_payload;
#define PBO_PAY_SZ sizeof(chan_pbo_payload)
typedef struct{
	u8 mode : 1;
	u8 val : 1;
} chan_pbo_changes;
#define PBO_CH_SZ sizeof(chan_pbo_changes)

// --------- Dump configuration -------------- //
#define DUMP_CFG_PAY_SZ 1
#define DUMP_CFG_CH_SZ 0


#endif



