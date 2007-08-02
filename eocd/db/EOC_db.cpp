/*
 * EOC_db.cpp
 *	EOC Data base unit, provide SHDSL channel information storage and 
 *	acces to it
 */
#include <generic/EOC_responses.h>
#include <generic/EOC_msg.h>
#include <db/EOC_db.h>
#include <eoc_debug.h>

int EOC_db::
register_handlers(){
    // register EOC responses handlers
    handlers[RESP_IND(RESP_DISCOVERY)] = _resp_discovery;
    handlers[RESP_IND(RESP_INVENTORY)] = _resp_inventory;
    handlers[RESP_IND(RESP_CONFIGURE)] = _resp_configure;
    handlers[RESP_IND(RESP_STATUS)] = _resp_status;
    handlers[RESP_IND(RESP_NSIDE_PERF)] = _resp_nside_perf;
    handlers[RESP_IND(RESP_CSIDE_PERF)] = _resp_cside_perf;
	
    // register Application requests for info
    app_handlers[app_frame::INVENTORY] = _appreq_inventory;
    app_handlers[app_frame::ENDP_CUR] = _appreq_endpcur;
    app_handlers[app_frame::ENDP_15MIN] = _appreq_endp15min;
    app_handlers[app_frame::ENDP_1DAY] = _appreq_endp1day;
    app_handlers[app_frame::ENDP_MAINT] = _appreq_endpmaint;
    app_handlers[app_frame::UNIT_MAINT] = _appreq_unitmaint;
}

EOC_db::
EOC_db(EOC_scheduler *s,int lnum){
    int i;
    for(i=0;i<MAX_UNITS;i++)
        units[i] = NULL;
    for(i=0;i<RESPONSE_QUAN;i++)
        handlers[i] = NULL;
    register_handlers();
    sch = s;
    loop_num = lnum;
}
    
int EOC_db::
response(EOC_msg *m)
{
    u8 type = RESP_IND(m->type());
//    printf("EOC_DB: Get response: %d\n",m->type());
    if( !m || !m->is_response() || !handlers[type] )
        return -1;
    return handlers[type](this,m);
}

// TODO: 
// what to do if inventory information of unit differs
int EOC_db::
add_unit(unit u, resp_inventory *resp)
{
    if( u<=unknown || u>(unit)MAX_UNITS )
        return -1;
    if( units[(int)u-1]){
        if( units[(int)u-1]->integrity(resp) ){
	    for(int i = (int)u-1;i<MAX_UNITS;i++){
		if( units[i] ){
		    delete units[i];
		    units[i] = NULL;
		}
	    }
	    if( units[(int)stu_r-1] ){
		delete units[(int)stu_r-1];
		units[(int)stu_r-1] = NULL;
	    }
	} else {
	    return 0;
	}
    }
    units[(int)u-1] = new EOC_unit(u,resp,loop_num);
}
    
int EOC_db::
clear(){
    for(int i=0; i<MAX_UNITS;i++){
        if( units[i] )
	    delete units[i];
    }
}

int EOC_db::
check_exist(unit u){
    u8 ind = (u8)u - 1;
    if( !(ind < MAX_UNITS) )
	return -1;
    if( units[(int)u - 1] )
        return 0;
    return -1;
}

int EOC_db::
check_exist(unit u,EOC_unit::Sides s){
    if( check_exist(u) )
        return -1;
    switch( s ){
    case EOC_unit::net_side:
        if( !units[(int)u-1]->nside() )
	    return 0;
	return -1;
    case EOC_unit::cust_side:
        if( !units[(int)u-1]->cside() )
	    return 0;
	return -1;
    }
    return -1;
}

int EOC_db::
check_exist(unit u,EOC_unit::Sides s,int loop)
{
    if( check_exist(u,s) )
        return -1;
    EOC_side *side;
    switch( s ){
    case EOC_unit::net_side:
        side = units[(int)u-1]->nside();
        break;
    case EOC_unit::cust_side:
        side = units[(int)u-1]->cside();
        break;
    }
    if( side->get_loop(loop) )
        return 0;
    return -1;
}

int EOC_db::
unit_quan(){
    int i,cnt=0;
    for(i=0;i<MAX_UNITS && units[i];i++){
        //printf("Count unit #%d\n",i);
        cnt++;
    }
    if( cnt < 2 ) return -1;
    return cnt;
}

//------------------- EOC responses -------------------------//

int EOC_db::
_resp_discovery(EOC_db *db,EOC_msg *m)
{
    ASSERT(m->type() == RESP_DISCOVERY );
    if( !db )
	return 0;
    resp_discovery *resp= (resp_discovery *)m->payload();
    printf("DISCOVERY_RESP FROM(%d): hop=%d,resl=%d,vendor_id=%d,fwd_loss=%d\n",
	    m->src(),resp->hop,resp->res1,resp->vendor_id,resp->fwd_loss);
    return 0;
}

int EOC_db::
_resp_inventory(EOC_db *db,EOC_msg *m)
{
    ASSERT( m->type() == RESP_INVENTORY);
    ASSERT( m->payload_sz() == RESP_INVENTORY_SZ);
    if( !db )
	return 0;

    printf("INVENTORY_RESP FROM(%d)\n",m->src());


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

int EOC_db::
_resp_configure(EOC_db *db,EOC_msg *m)
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

int EOC_db::
_resp_status(EOC_db *db,EOC_msg *m)
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

int EOC_db::
_resp_nside_perf(EOC_db *db,EOC_msg *m)
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


int EOC_db::
_resp_cside_perf(EOC_db *db,EOC_msg *m)
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



int EOC_db::
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

// ------------ Application requests ------------------------------//
int EOC_db::
app_request(app_frame *fr)
{
    if( !app_handlers[fr->id()] )
	return -1;
    return app_handlers[fr->id()](this,fr);
}

int EOC_db::
_appreq_inventory(EOC_db *db,app_frame *fr)
{
    inventory_payload *p = (inventory_payload*)fr->payload_ptr();
    printf("DB: Inventory app request\n");
    if( !p ){
	printf("DB Inventory: eror !p\n");    
	return -1;
    }
    if( db->check_exist((unit)p->unit) ){
	printf("DB Inventory: error check exist\n");	
	fr->negative();
	return 0;
    }
    printf("DB Inventory: form response\n");	    
    fr->response();
    printf("DB Inventory: prisvaivanie\n");	    
    p->inv = db->units[p->unit-1]->inventory_info();
    p->region1 = 1;
    p->region0 = 1;
    printf("DB Inventory: success\n");	    
    return 0;
}

int EOC_db::
_appreq_endpcur(EOC_db *db,app_frame *fr)
{
    fr->response();
    return 0;
}

int EOC_db::
_appreq_endp15min(EOC_db *db,app_frame *fr)
{
    fr->response();
    return 0;
}

int EOC_db::
_appreq_endp1day(EOC_db *db,app_frame *fr)
{
    fr->response();
    return 0;
}

int EOC_db::
_appreq_endpmaint(EOC_db *db,app_frame *fr)
{
    fr->response();
    return 0;
}

int EOC_db::
_appreq_unitmaint(EOC_db *db,app_frame *fr)
{
    fr->response();
    return 0;
}

