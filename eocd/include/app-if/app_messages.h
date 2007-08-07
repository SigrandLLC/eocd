#ifndef APP_MESSAGES_H
#define APP_MESSAGES_H

#include <snmp/snmp-generic.h>
#include <span_profile.h>
#include <generic/EOC_responses.h>
#include <generic/EOC_types.h>
typedef struct{
    s16 nreps;
    char conf_prof[SNMP_ADMIN_LEN+1];
    char alarm_prof[SNMP_ADMIN_LEN+1];
} span_conf_payload;
#define SPAN_CONF_PAY_SZ sizeof(span_conf_payload)
#define SPAN_CONF_CH_SZ 0


typedef struct{
    u32 nreps;
    u32 max_lrate;
    u32 act_lrate;
    u8 region0 : 1;
    u8 region1 : 1;
    u8 : 6;
    u32 max_prate;
    u32 act_prate;
} span_status_payload;
#define SPAN_STATUS_PAY_SZ sizeof(span_status_payload)
#define SPAN_STATUS_CH_SZ 0

typedef struct{
    u8 unit;
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

typedef struct{
    char ProfileName[SNMP_ADMIN_LEN+1];
    span_conf_profile_t conf;
} span_conf_prof_payload;
#define SPAN_CONF_PROF_PAY_SZ sizeof(span_conf_prof_payload)

typedef struct{
    u8 annex :1;
    u8 wires :1;      
    u8 power :1;
    u8 psd :1;
    u8 clk:1;
    u8 line_probe:1;
    u8 remote_cfg:1;
    u8 currCondDown :1;                                                   
    u8 worstCaseDown :1;                                                  
    u8 currCondUp :1;                                                     
    u8 worstCaseUp :1;
    u8 min_rate:1;
    u8 max_rate:1;
    s8 cur_marg_down:1;
    s8 worst_marg_down:1;
    s8 cur_marg_up:1;
    s8 worst_marg_up:1;
} span_conf_prof_changes;
#define SPAN_CONF_PROF_CH_SZ sizeof(span_conf_prof_changes)

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

#endif
