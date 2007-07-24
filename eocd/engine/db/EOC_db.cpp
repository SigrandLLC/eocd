/*
 * EOC_poller_resp.cpp
 *	EOC response process functions
 *	Prototype:
 *		int (*response_handler_t)(EOC_db *d,EOC_msg *m);
 */
#include <generic/EOC_responses.h>
#include <generic/EOC_msg.h>
#include <db/EOC_db.h>
#include <eoc_debug.h>

int
EOC_db::_resp_discovery(EOC_db *db,EOC_msg *m)
{
    ASSERT(m->type() == RESP_DISCOVERY );
    if( !db )
	return 0;
    resp_discovery *resp= (resp_discovery *)m->payload();
    printf("DISCOVERY_RESP FROM(%d): hop=%d,resl=%d,vendor_id=%d,fwd_loss=%d\n",
	    m->src(),resp->hop,resp->res1,resp->vendor_id,resp->fwd_loss);
    return 0;
}

int
EOC_db::_resp_inventory(EOC_db *db,EOC_msg *m)
{
    ASSERT( m->type() == RESP_INVENTORY);
    ASSERT( m->payload_sz() == RESP_INVENTORY_SZ);
    if( !db )
	return 0;

    resp_inventory *resp= (resp_inventory *)m->payload();
    int ind = (int)m->src() - 1;
    if( !db->units[ind] ){
	db->units[ind] = new EOC_unit(m->src(),resp,db->loop_num);
    }else{
	// Check that units was not changed
	if(db->units[ind]->integrity(resp) ){
	    for(int i=ind;i<MAX_UNITS;i++){
		if( db->units[i] ){
		    delete db->units[i];
		}
	    }
	}
    }
    return 0;
}

int
EOC_db::_resp_configure(EOC_db *db,EOC_msg *m)
{
    ASSERT( m->type() == RESP_CONFIGURE );
    ASSERT( m->payload_sz() == RESP_CONFIGURE_SZ);
    if( !db )
	return 0;

    resp_configure *resp= (resp_configure *)m->payload();
    printf("CONFIGURE_RESP FROM(%d), loop(%d),snr(%d)\n",m->src(),resp->loop_attn,resp->snr_marg);
    // db->add_unit(m->src(),resp);
    return 0;
}

int
EOC_db::_resp_status(EOC_db *db,EOC_msg *m)
{
    ASSERT( m->type() == RESP_STATUS );
    ASSERT( m->payload_sz() == RESP_STATUS_SZ);
//    if( m->payload_sz() == RESP_STATUS_SZ ){
//	printf("\n");
//    }
    
    ASSERT(m);
    printf("STATUS RESPONSE: src(%d) dst(%d)\n",m->src(),m->dst());
    resp_status *resp= (resp_status*)m->payload();
    int loop_id = resp->loop_id-1;
    EOC_loop *nsloop=NULL,*csloop=NULL;
    int ind = (int)m->src() - 1;

    if( !db->units[ind] ){
	// TODO eoc_log(LOG_ERROR,"Status message from unit which not exist");
	return -1;
    }
    
    if( db->units[ind]->cside() ){
	if( !db->units[ind]->cside()->get_loop(loop_id) ){
	    // TODO eoc_log(LOG_ERROR,"(Status message) Request unexisted loop");
	    return -1;
	}
	csloop = db->units[ind]->cside()->get_loop(loop_id);
    }

    if( db->units[ind]->nside() ){
	if( !db->units[ind]->nside()->get_loop(loop_id) ){
	    // TODO eoc_log(LOG_ERROR,"(Status message) Request unexisted loop");
	    return -1;
	}
	nsloop = db->units[ind]->nside()->get_loop(loop_id);
    }

    if( !db )
	return 0;

    if( csloop ){
	csloop->short_status(resp->cs_snr_marg);
    }
    if( nsloop ){
	nsloop->short_status(resp->ns_snr_marg);
    }

    return 0;
}

int
EOC_db::_resp_nside_perf(EOC_db *db,EOC_msg *m)
{
    ASSERT( m->type() == RESP_NSIDE_PERF );
    ASSERT( m->payload_sz() == RESP_NSIDE_PERF_SZ);
    ASSERT(m && db);
    resp_nside_perf *resp= (resp_nside_perf*)m->payload();
    int loop_id = resp->loop_id-1; 
    EOC_loop *nsloop=NULL;
    int ind = (int)m->src() - 1;    

    printf("NET SIDE PERF RESPONSE: src(%d) dst(%d)\n",m->src(),m->dst());

    if( !db->units[ind] ){
	// TODO eoc_log(LOG_ERROR,"Network side perfomance message from unit which not exist");
	printf("Network side perfomance message from unit which not exist\n");
	return -1;
    }
    
    if( db->units[ind]->nside() ){
	if( !db->units[ind]->nside()->get_loop(loop_id) ){
	    // TODO eoc_log(LOG_ERROR,"(Network side perfomance) Request unexisted loop");
	    printf("(Network side perfomance) Request unexisted loop\n");
	    return -1;
	}
	nsloop = db->units[ind]->nside()->get_loop(loop_id);
    }else{
	// TODO eoc_log(LOG_ERROR,"(Network side perf status message) Request unexisted side");
	printf("(Network side perf status message) Request unexisted side\n");
	return -1;
    }

    if( !db )
	return 0;
    
    if( nsloop ){
	nsloop->full_status(resp);
    }
    return 0;
}


int
EOC_db::_resp_cside_perf(EOC_db *db,EOC_msg *m)
{
    ASSERT( m->type() == RESP_CSIDE_PERF );
    ASSERT( m->payload_sz() == RESP_CSIDE_PERF_SZ);
    ASSERT(m);
    resp_cside_perf *resp= (resp_cside_perf*)m->payload();
    int loop_id = resp->loop_id-1;
    EOC_loop *csloop=NULL;
    int ind = (int)m->src() - 1;    

    printf("CUST SIDE PERF RESPONSE: src(%d) dst(%d)\n",m->src(),m->dst());


    if( !db->units[ind] ){
	// TODO eoc_log(LOG_ERROR,"Customer side perf status message from unit which not exist");
	printf("Customer side perf status message from unit which not exist\n");
	return -1;
    }
    
    if( db->units[ind]->cside() ){
	if( !db->units[ind]->cside()->get_loop(loop_id) ){
	    // TODO eoc_log(LOG_ERROR,"(Customer side perf status message) Request unexisted loop");
	    printf("(Customer side perf status message) Request unexisted loop\n");
	    return -1;
	}
	csloop = db->units[ind]->cside()->get_loop(loop_id);
    }else{
	// TODO eoc_log(LOG_ERROR,"(Customer side perf status message) Request unexisted side");
	printf("(Customer side perf status message) Request unexisted side\n");
	return -1;
    }

    if( !db )
	return 0;
    
    if( csloop ){
	csloop->full_status(resp);
    }
    return 0;
}



int
EOC_db::_resp_test(EOC_db *db,EOC_msg *m)
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
