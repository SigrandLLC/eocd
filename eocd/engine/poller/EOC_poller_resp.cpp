/*
 * EOC_poller_resp.cpp
 *	EOC response process functions
 *	Prototype:
 *		int (*response_handler_t)(EOC_db *d,EOC_msg *m);
 */
#include <generic/EOC_responses.h>
#include <generic/EOC_msg.h>
#include <db/EOC_db.h>

int
_resp_discovery(EOC_db *db,EOC_msg *m)
{
    if( m->type() != RESP_DISCOVERY ){
//	PDEBUG(POLLER_RESP,"Wrong message type: %d",m->(type()) );
	return -1;
    }
    if( !db )
	return 0;
    resp_discovery *resp= (resp_discovery *)m->payload();
    printf("DISCOVERY_RESP FROM(%d): hop=%d,resl=%d,vendor_id=%d,fwd_loss=%d\n",
	    m->src(),resp->hop,resp->res1,resp->vendor_id,resp->fwd_loss);
    // db->add_unit(m->src(),resp);
    return 0;
}

int
_resp_inventory(EOC_db *db,EOC_msg *m)
{
    if( m->type() != RESP_INVENTORY ){
//	PDEBUG(POLLER_RESP,"Wrong message type: %d",m->(type()) );
	return -1;
    }

    if( !db )
	return 0;

    resp_inventory *resp= (resp_inventory *)m->payload();
    printf("INVENTORY_RESP FROM(%d): shdsl_ver(%d)\n",m->src(),resp->shdsl_ver);
    fflush(stdout);
    // db->add_unit(m->src(),resp);
    return 0;
}

int
_resp_configure(EOC_db *db,EOC_msg *m)
{
    if( m->type() != RESP_CONFIGURE ){
//	PDEBUG(POLLER_RESP,"Wrong message type: %d",m->(type()) );
	return -1;
    }

    if( !db )
	return 0;

    resp_configure *resp= (resp_configure *)m->payload();
    printf("CONFIGURE_RESP FROM(%d), loop(%d),snr(%d)\n",m->src(),resp->loop_attn,resp->snr_marg);
    // db->add_unit(m->src(),resp);
    return 0;
}

int
_resp_test(EOC_db *db,EOC_msg *m)
{
    if( m->type() != 15+128 ){
	return -1;
    }

    if( !db )
	return 0;

    resp_configure *resp= (resp_configure *)m->payload();
    printf("TEST_RESP FROM(%d), loop(%d),snr(%d)\n",m->src(),resp->loop_attn,resp->snr_marg);
    // db->add_unit(m->src(),resp);
    return 0;
}
