/*
 * EOC_responses.h:
 *	Contains structures of SHDSL EOC responses
 */

#ifndef EOC_RESPONSES_H
#define EOC_RESPONSES_H
#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <generic/EOC_requests.h>

#define REQ2RESP(x) (x + RESP_OFFSET)
#define RESP2REQ(x) (x - RESP_OFFSET)
#define RESP_IND(x) (x - RESP_OFFSET-1)

// DISCOVERY response
typedef struct{
    u8 hop;
    u8 res1;
    u8 vendor_id[8];
    u8 eoc_softw_ver;
    u8 shdsl_ver;
    u8 fwd_loss:1;
    u8 :7;
} resp_discovery;
#define RESP_DISCOVERY REQ2RESP(REQ_DISCOVERY)
#define RESP_DISCOVERY_SZ sizeof(resp_discovery)

// IVENTORY response
typedef struct{
    u8 shdsl_ver;
    u8 ven_lst[3];
    u8 ven_issue[2];
    u8 softw_ver[6];
    u8 unit_id_code[10];
    u8 res1;
    u8 ven_id[8];
    u8 ven_model[12];
    u8 ven_serial[12];
    u8 other[12];
} resp_inventory;
#define RESP_INVENTORY REQ2RESP(REQ_INVENTORY)
#define RESP_INVENTORY_SZ sizeof(resp_inventory)


// CONFIGURE response
typedef struct{
    u8 utc:1;
    u8 :7;
    u8 loop_attn;
    u8 :4;
    u8 snr_marg :4;
} resp_configure;
#define RESP_CONFIGURE REQ2RESP(REQ_CONFIGURE)
#define RESP_CONFIGURE_SZ sizeof(resp_configure)

// STATUS response
typedef struct{
    s8 ns_snr_marg;
    s8 cs_snr_marg;
    u8 loop_id;
} resp_status;
#define RESP_STATUS REQ2RESP(REQ_STATUS)
#define RESP_STATUS_SZ sizeof(resp_status)

// SIDE PERF STATUS response
typedef struct{
    u8 :1;
    u8 losws_alarm :1;
    u8 loop_attn_alarm:1;
    u8 snr_marg_alarm :1;
    u8 dc_cont_flt:1;
    u8 dev_flt:1;
    u8 pwr_bckoff_st:1;
    u8 :1;
    s8 snr_marg;
    s8 loop_attn;
    u8 es;
    u8 ses;
    u16 crc;
    u8 losws;
    u8 uas;
    u8 pwr_bckoff_base_val:4;
    u8 cntr_rst_scur:1;
    u8 cntr_ovfl_stur:1;
    u8 cntr_rst_scuc:1;
    u8 cntr_ovfl_stuc:1;
    u8 loop_id:3;
    u8 :4;
    u8 pwr_bkf_ext:1;
}side_perf;
typedef side_perf resp_cside_perf;
typedef side_perf resp_nside_perf;
#define RESP_NSIDE_PERF 140
#define RESP_NSIDE_PERF_SZ sizeof(resp_cside_perf)
#define RESP_CSIDE_PERF 141
#define RESP_CSIDE_PERF_SZ sizeof(resp_cside_perf)

#define RESP_MAINT_STAT 137
/*TODO : Add structure */

#endif
