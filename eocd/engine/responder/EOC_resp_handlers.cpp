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

EOC_msg *get_status(EOC_dev *dev,int loop){
    return new EOC_msg;
}

int
EOC_responder::_inventory(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    assert( in && m );
    assert( m->type() == REQ_INVENTORY );
    ret = NULL;
    cnt = 1;
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
EOC_responder::_configure(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    assert( in && m );
    assert(m->type() == REQ_CONFIGURE );
    EOC_router *r = in->r;
    req_configure *req = (req_configure *)m->payload();
    u8 normal = req->conf_type;
    u8 loop = req->loop_attn;
    u8 snr = req->snr_marg;
    cnt = 1;
    ret = NULL;
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
EOC_responder::_status(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    assert( in && m );
    assert(m->type() == REQ_STATUS );
    EOC_router *r = in->r;
    EOC_dev *cs = r->nsdev();
    EOC_dev *ns = r->nsdev();
    u8 cs_loops = (cs) ? cs->loops() : 0; 
    u8 ns_loops = (ns) ? ns->loops() : 0;
    assert(cs_loops == ns_loops);
    assert(cs_loops <= MAX_LOOPS );
    int loop,offs=0;
    EOC_msg **array = new EOC_msg*[(MAX_LOOPS*4)*2];
    int msgs_num = 0;
    
    // Generate status responses
    m->response(RESP_STATUS_SZ);
    for(loop=0;loop<ns_loops;loop++){
	array[loop+offs] = new EOC_msg(m);
	resp_status *resp = (resp_status *)m->payload();
	memset(resp,0,RESP_STATUS_SZ);
	if( ns ){
	    resp->ns_snr_marg = ns->snr(loop);
	}
	if( cs ){
	    resp->cs_snr_marg = cs->snr(loop);
	}
    }
    offs += loop;
    // Setup 
    for(loop=0;loop<cs_loops;loop++){
	if( cs->perf_change(loop) ){
	    array[loop+offs] = get_status(cs,loop);
	}
    }    
    offs += loop;
    // allocate array of messages
    for(loop=0;loop<ns_loops;loop++){
	if( ns->perf_change(loop) ){
	    array[loop+offs] = get_status(ns,loop);
	}
    }    
    offs += loop;
    cnt = offs;
    ret = array;
    return 0;
}


int
EOC_responder::_test(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    if( m->type() != 15 )
	return -1;
    printf("TEST_REQUEST getted\n");
    m->response(10);
    return 0;
}
