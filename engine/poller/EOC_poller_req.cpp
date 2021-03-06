/*
 * EOC_poller_req:
 *	EOC requests generation handlers.
 *	Requests generated by type and destination address
 *	Prototype:
 *	    EOC_msg *(*request_handler_t)(req_mode mode,unit src,unit dst,unsigned char type);
 */
#include <generic/EOC_requests.h>
#include <generic/EOC_msg.h>
#include <db/EOC_db.h>
#include <engine/EOC_handlers.h>
#include <eoc_debug.h>

EOC_msg *
_req_discovery(sched_state stat,sched_elem el,EOC_config *cfg)
{
    ASSERT( el.type == REQ_DISCOVERY );

    EOC_msg *m = new EOC_msg(REQ_DISCOVERY_SZ);
    m->msize();
    // TODO: make as exception!!!
    if( !m->mptr() )
		return NULL;
    req_discovery *req = (req_discovery *)m->payload();
    req->hop = 0;
    m->src(el.src);
    m->dst(el.dst);
    m->type(REQ_DISCOVERY);
    return m;
}


EOC_msg *
_req_inventory(sched_state stat,sched_elem el,EOC_config *cfg)
{
    ASSERT( el.type == REQ_INVENTORY );

    EOC_msg *m = new EOC_msg(REQ_INVENTORY_SZ);
    // TODO: make as exception!!!
    if( !m->mptr() )
		return NULL;
    req_inventory *req = (req_inventory *)m->payload();
    m->src(el.src);
    m->dst(el.dst);
    m->type(REQ_INVENTORY);
    return m;
}

EOC_msg *
_req_configure(sched_state stat,sched_elem el,EOC_config *cfg)
{
    ASSERT( el.type == REQ_CONFIGURE );

    EOC_msg *m = new EOC_msg(REQ_CONFIGURE_SZ);
    // TODO: make as exception!!!
    if( !m->mptr() )
		return NULL;
    req_configure *req = (req_configure *)m->payload();
    m->src(el.src);
    m->dst(el.dst);
    m->type(REQ_CONFIGURE);
    switch( stat ){
    case EOC_scheduler::Setup:
		req->conf_type = 0;
    case EOC_scheduler::Normal:
		req->conf_type = 1;
    }
    req->loop_attn = cfg->loop_tresh();
    req->snr_marg = cfg->snr_tresh();
    return m;
}

EOC_msg *
_req_status(sched_state stat,sched_elem el,EOC_config *cfg)
{
    ASSERT( el.type == REQ_STATUS );
    EOC_msg *m = new EOC_msg(REQ_STATUS_SZ);
    // TODO: make as exception!!!
    if( !m->mptr() )
		return NULL;
    char *req = (char *)m->payload();
    m->src(el.src);
    m->dst(el.dst);
    m->type(REQ_STATUS);
    return m;
}

EOC_msg *
_req_test(sched_state stat,sched_elem el,EOC_config *cfg)
{
    if( el.type != 15 )
		return NULL;
    EOC_msg *m = new EOC_msg(10);
    // TODO: make as exception!!!
    if( !m->mptr() )
		return NULL;
    char *req = (char *)m->payload();
    m->src(el.src);
    m->dst(el.dst);
    m->type(15);
    memcpy(req,"aaaaaaaaaaaaaaa",10);
    return m;
}



