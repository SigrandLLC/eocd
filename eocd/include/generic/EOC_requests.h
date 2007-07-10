/*
 * EOC_requests.h:
 *	Contains structures of SHDSL EOC requests
 */
#ifndef EOC_REQUESTS_H
#define EOC_REQUESTS_H

#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>

#define REQ_DISCOVERY 1
typedef struct{
    u8 hop;
} req_discovery; 
#define REQ_DISCOVERY_SZ sizeof(req_discovery)

#define REQ_INVENTORY 2
typedef struct{
}req_inventory; 
#define REQ_INVENTORY_SZ sizeof(req_inventory)

#define REQ_CONFIGURE 3
typedef struct{
    u8 loop_attn:7;
    u8 conf_type:1;
    u8 :4;
    u8 snr_marg:4;
} req_configure;
#define REQ_CONFIGURE_SZ sizeof(req_configure)



#endif
