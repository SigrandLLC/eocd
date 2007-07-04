/*
 * EOC_poller_resp.cpp
 *	EOC response process functions
 *	Prototype:
 *		int (*response_handler_t)(EOC_db *d,EOC_msg *m);
 */
#include <generic/EOC_response.h>
#include <generic/EOC_msg.h>
#include <db/EOC_db.h>


int
_resp_discovery(EOC_db *db,EOC_msg *m)
{
    if( m->type() != RESP_DISCOVERY ){
	PDEBUG(POLLER_RESP,"Wrong message type: %d",m->(type()) );
	return -1;
    }
    resp_discovery *resp= (resp_discovery *)(m->payload();
    printf("Get from %d: hop=%d,resl=%d,vendor_id=%d,fwd_loss=%d",
	    m->src(),resp->hop,resp->resl,resp->vendor_id,resp->fws_loss);
    // db->add_unit(m->src(),resp);
    return 0;
}

