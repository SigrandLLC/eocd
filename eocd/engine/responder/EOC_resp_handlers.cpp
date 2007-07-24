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
get_status(EOC_dev *dev,int loop,EOC_msg *m)
{
    side_perf *perf = (side_perf*)m->payload();
    perf->losws_alarm = dev->losws_alarm(loop);
    perf->loop_attn_alarm = dev->loop_attn_alarm(loop);
    perf->snr_marg_alarm = dev->snr_marg_alarm(loop);
    perf->dc_cont_flt = dev->dc_cont_flt(loop);
    perf->dev_flt = dev->dev_flt(loop);
    perf->pwr_bckoff_st = dev->pwr_bckoff_st(loop);
    perf->snr_marg = dev->snr_marg(loop);
    perf->loop_attn = dev->loop_attn(loop);
    perf->es = dev->es(loop);
    perf->ses = dev->ses(loop);
    perf->crc = dev->crc(loop);
    perf->losws = dev->losws(loop);
    perf->uas = dev->uas(loop);
    perf->pwr_bckoff_base_val = dev->pwr_bckoff_base_val(loop);
    perf->cntr_rst_scur = dev->cntr_rst_scur(loop);
    perf->cntr_ovfl_stur = dev->cntr_ovfl_stur(loop);
    perf->cntr_rst_scuc = dev->cntr_rst_scuc(loop);
    perf->cntr_ovfl_stuc = dev->cntr_ovfl_stuc(loop);
    perf->loop_id = loop+1;
    perf->pwr_bkf_ext = dev->pwr_bkf_ext(loop);
    return 0;
}


int
EOC_responder::_status(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    assert( in && m );
    assert(m->type() == REQ_STATUS );
    EOC_router *r = in->r;
    EOC_dev *ns = r->nsdev();
    EOC_dev *cs = r->csdev();
    int loop_num = r->loops();
    int loop,offs=0;
    EOC_msg **array = new EOC_msg*[(loop_num*4)*2];

    for(int i=0;i<(loop_num*4)*2;i++){
	array[i] = NULL;
    }

    if( cs )
	cs->status_collect();
    if( ns )
	ns->status_collect();
    
    // Generate status responses
    m->response(RESP_STATUS_SZ);
    for(loop=0;loop<loop_num;loop++){
	array[loop] = new EOC_msg(m);
	resp_status *resp = (resp_status *)array[loop+offs]->payload();
	memset(resp,0,RESP_STATUS_SZ);
	if( ns ){
	    resp->ns_snr_marg = ns->snr_marg(loop);
	}
	if( cs ){
	    resp->cs_snr_marg = cs->snr_marg(loop);
	}
	resp->loop_id = loop+1;
    }
    offs += loop;
    // Setup
    if( cs ){
	for(loop=0;loop<loop_num;loop++){
	    if( cs->perf_change(loop) ){
		if( !(array[offs] = new EOC_msg(m,SIDE_PERF_SZ)) )
		    goto err_exit;
		EOC_msg *t = array[offs];
		t->type(RESP_CSIDE_PERF);
		offs++;
		if( get_status(cs,loop,t) )
		    goto err_exit;
	    }
	}
    }	    
    // network side
    if( ns ){
	for(loop=0;loop<loop_num;loop++){
	    if( ns->perf_change(loop) ){
		if( !(array[offs] = new EOC_msg(m,SIDE_PERF_SZ)) )
		    goto err_exit;
		EOC_msg *t = array[offs];
		t->type(RESP_NSIDE_PERF);
		offs++;    
		if( get_status(ns,loop,t) )
		    goto err_exit;
	    }
	}
    }	    

    cnt = offs;
    ret = array;
    return 0;
err_exit:
    for(int i=0;i<offs;i++){
	if( array[i] )
	    delete array[i];
    }
    delete[] array;
    ret = NULL;
    return -1;
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
