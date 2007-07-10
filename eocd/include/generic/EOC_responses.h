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



#endif
