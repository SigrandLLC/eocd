/*
 * EOC_resp_handlers.cpp:
 *	Contains handlers for EOC requests on Regenerators (SRUx) 
 *	& Terminators (STU-C,STU-R)
 *	Prototype:
 *		EOC_msg *(*responder_handler_t)(EOC_dev *dev,EOC_msg *m);
 */  

#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/EOC_handlers.h>
#include <engine/EOC_router.h>

int
_inventory(EOC_router *r,EOC_msg *m)
{
    if( m->type() != REQ_INVENTORY )
	return -1;
    m->response(RESP_INVENTORY_SZ);
    resp_inventory *resp = (resp_inventory *)m->payload();
    memset(resp,0,RESP_INVENTORY_SZ);
    resp->shdsl_ver = 0x08;
    memcpy(resp->ven_lst,"001",3); // Hardware version
    memcpy(resp->ven_issue,"01",2); // Usage of the unit
    memcpy(resp->softw_ver,"000001",6); // Software version 
//    memcpy(resp->unit_id_code,"001",3);
    memcpy(resp->ven_id,"Sigrand",7);    
    memcpy(resp->ven_model,"001",3);
    memcpy(resp->ven_serial,"001",3);
    return 0;
}

int
_configure(EOC_router *r,EOC_msg *m)
{
    if( m->type() != REQ_CONFIGURE )
	return -1;
    req_configure *req = (req_configure *)m->payload();
    u8 normal = req->conf_type;
    u8 loop = req->loop_attn;
    u8 snr = req->snr_marg;
    req = NULL;
    m->response(RESP_CONFIGURE_SZ);
    resp_configure *resp = (resp_configure *)m->payload();
    memset(resp,0,RESP_CONFIGURE_SZ);
    if( normal ){
	if( r->csdev() && r->csdev()->tresholds(loop,snr) )
	    resp->utc = 1;
	if( r->nsdev() && r->nsdev()->tresholds(loop,snr) )
	    resp->utc = 1;
    }
    resp->loop_attn = loop;
    resp->snr_marg = snr; 
    return 0;
}

int
_test(EOC_router *r,EOC_msg *m)
{
    if( m->type() != 15 )
	return -1;
    printf("TEST_REQUEST getted\n");
    m->response(10);
    return 0;
}
