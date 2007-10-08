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
    handlers[RESP_IND(RESP_SENSOR_STATE)] = _resp_sensor_state;
	
    // register Application requests for info
    app_handlers[APP_INVENTORY] = _appreq_inventory;
    app_handlers[APP_ENDP_CUR] = _appreq_endpcur;
    app_handlers[APP_ENDP_15MIN] = _appreq_endp15min;
    app_handlers[APP_ENDP_1DAY] = _appreq_endp1day;
    app_handlers[APP_ENDP_MAINT] = _appreq_endpmaint;
    app_handlers[APP_UNIT_MAINT] = _appreq_unitmaint;
}

EOC_db::
EOC_db(EOC_scheduler *s,int lnum){
    int i;
    for(i=0;i<MAX_UNITS;i++){
        units[i] = NULL;
        units_discov[i] = 0;
    }
	
    for(i=0;i<RESPONSE_QUAN;i++)
        handlers[i] = NULL;
    register_handlers();
    sch = s;
    loop_num = lnum;
}
    
int EOC_db::
response(EOC_msg *m,int check)
{
    u8 type = RESP_IND(m->type());
    if( !m || !m->is_response() || !handlers[type] )
        return -1;
    return handlers[type](this,m,check);
}

// TODO: 
// what to do if inventory information of unit differs
/*
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
*/

    
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
    if( units[(int)u - 1] && units_discov[(int)u - 1] )
        return 0;
    return -1;
}

EOC_side *EOC_db::
check_exist(unit u,side s){
    if( check_exist(u) )
        return NULL;
    switch( s ){
    case net_side:
        return units[(int)u-1]->nside();
    case cust_side:
        return units[(int)u-1]->cside();
    }
    return NULL;
}

EOC_loop *EOC_db::
check_exist(unit u,side s,int loop)
{
    EOC_side *side = check_exist(u,s);
    if( !side )
	return NULL;
    return side->get_loop(loop);
}

int EOC_db::
unit_quan(){
    int i,cnt=0;
    for(i=0;i<MAX_UNITS ;i++){
	if( units_discov[i] )
    	    cnt++;
    }
    return cnt;
}

int EOC_db::
reg_quan(){
    int i,cnt=0;
    for(i=2;i<MAX_UNITS ;i++){
	if( units_discov[i] )
    	    cnt++;
    }
    return cnt;
}

int EOC_db::
link_established()
{
    if( units_discov[1] ){
    	return 1;
    }
    return 0;
}


void EOC_db::
link_down()
{
    PDEBUG(DERR,"Link down");
    for(int i=0;i<MAX_UNITS;i++)
	units_discov[i] = 0;
}

//------------------- EOC responses -------------------------//

int EOC_db::
_resp_discovery(EOC_db *db,EOC_msg *m,int check)
{
    ASSERT(m->type() == RESP_DISCOVERY );

    resp_discovery *resp= (resp_discovery *)m->payload();
    int ind = (int)m->src() - 1;
    if( check )
	return 0;
	
    if( !db->units[ind] ){
	PDEBUG(DERR,"Add new unit %d",ind);
	db->units[ind] = new EOC_unit(m->src(),resp,db->loop_num);
    }
    db->units_discov[ind] = 1;
    PDEBUG(DINFO,"DISCOVERY_RESP FROM(%d): hop=%d,resl=%d,vendor_id=%d,fwd_loss=%d",
	    m->src(),resp->hop,resp->res1,resp->vendor_id,resp->fwd_loss);
    if( m->src() == stu_r ){
	PDEBUG(DERR,"Clean units list");
	for(int i=0;i<MAX_UNITS;i++){
	    if( db->units[i] && !db->units_discov[i] ){
		PDEBUG(DERR,"Unit(%d) is not present anymore\n",i);
		delete db->units[i];
		db->units[i] = NULL;
	    }
	}
    }
    return 0;
}

int EOC_db::
_resp_inventory(EOC_db *db,EOC_msg *m,int check)
{
    ASSERT( m->type() == RESP_INVENTORY);
    ASSERT( m->payload_sz() == RESP_INVENTORY_SZ);

    resp_inventory *resp= (resp_inventory *)m->payload();

    int ind = (int)m->src() - 1;
    if( !db->units[ind] ){
	return -1;
    }
    
    if( check )
	return 0;

    PDEBUG(DINFO,"INVENTORY_RESP FROM(%d)",m->src());

    // Check that units was not changed
    if( db->units[ind]->integrity(resp) ){
	for(int i=ind+1;i<MAX_UNITS;i++){
	    if( db->units[i] ){
	        delete db->units[i];
	    }
	}
    }
    db->units[ind]->set_inv_info(resp);
    
    return 0;
}

int EOC_db::
_resp_configure(EOC_db *db,EOC_msg *m,int check)
{
    ASSERT( m->type() == RESP_CONFIGURE );
    ASSERT( m->payload_sz() == RESP_CONFIGURE_SZ);

    resp_configure *resp= (resp_configure *)m->payload();
    if( check )
	return 0;
    PDEBUG(DINFO,"CONFIGURE_RESP FROM(%d), loop(%d),snr(%d)",m->src(),resp->loop_attn,resp->snr_marg);
    return 0;
}

int EOC_db::
_resp_status(EOC_db *db,EOC_msg *m,int check)
{
    ASSERT( m->type() == RESP_STATUS );
    ASSERT( m->payload_sz() == RESP_STATUS_SZ);
    ASSERT(m);

    resp_status *resp= (resp_status*)m->payload();
    int loop_id = resp->loop_id-1;
    EOC_loop *nsloop=NULL,*csloop=NULL;

    nsloop = db->check_exist(m->src(),net_side,loop_id);
    csloop = db->check_exist(m->src(),cust_side,loop_id);
    
    switch(m->src()){
    case stu_c:
	if( !csloop )
	    return -1;
	break;
    case stu_r:
	if( !nsloop )
	    return -1;
	break;
    default:
	if( !csloop || !nsloop )
	    return -1;
	break;
    }

    if( check )
	return 0;

    PDEBUG(DINFO,"STATUS RESPONSE: src(%d) dst(%d) ns_snr=%d, cs_snr=%d",
	    m->src(),m->dst(),resp->ns_snr_marg,resp->cs_snr_marg);


    if( csloop ){
	csloop->short_status(resp->cs_snr_marg);
    }
    if( nsloop ){
	nsloop->short_status(resp->ns_snr_marg);
    }

    return 0;
}

int EOC_db::
_resp_nside_perf(EOC_db *db,EOC_msg *m,int check)
{
    ASSERT( m->type() == RESP_NSIDE_PERF );
    ASSERT( m->payload_sz() == RESP_NSIDE_PERF_SZ);
    ASSERT(m && db);
    resp_nside_perf *resp= (resp_nside_perf*)m->payload();
    int loop_id = resp->loop_id-1; 
    EOC_loop *nsloop=NULL;

    if( !(nsloop = db->check_exist(m->src(),net_side,loop_id) ) )
	return -1;
    
    if( check )
	return 0;

    PDEBUG(DINFO,"NET SIDE PERF RESPONSE: src(%d) dst(%d)",m->src(),m->dst());
    
    if( nsloop ){
	nsloop->full_status(resp);
    }
    return 0;
}


int EOC_db::
_resp_cside_perf(EOC_db *db,EOC_msg *m,int check)
{
    ASSERT( m->type() == RESP_CSIDE_PERF );
    ASSERT( m->payload_sz() == RESP_CSIDE_PERF_SZ);
    ASSERT(m);
    resp_cside_perf *resp= (resp_cside_perf*)m->payload();
    int loop_id = resp->loop_id-1;
    EOC_loop *csloop=NULL;


    if( !(csloop = db->check_exist(m->src(),cust_side,loop_id)) )
	return -1;
	
    if( check )
	return 0;

    PDEBUG(DINFO,"CUST SIDE PERF RESPONSE: src(%d) dst(%d)",m->src(),m->dst());
    
    if( csloop ){
	csloop->full_status(resp);
    }
    return 0;
}

int EOC_db::
_resp_sensor_state(EOC_db *db,EOC_msg *m,int check)
{
    ASSERT( m->type() == RESP_SENSOR_STATE );
    ASSERT( m->payload_sz() == RESP_SENSOR_STATE_SZ);
    ASSERT(m);
    resp_sensor_state *resp= (resp_sensor_state*)m->payload();
    EOC_unit *unit=NULL;

    if( db->check_exist(m->src()) )
	return -1;

    if( check )
	return 0;

    unit = db->units[(int)m->src()-1];
    if( unit ){
	PDEBUG(DINFO,"SENSOR STATE: src(%d): s1(%d), s2(%d), s3(%d)",m->src(),resp->sensor1,resp->sensor2,resp->sensor3);
	unit->sensor_resp(resp);
    }
    return 0;
}


int EOC_db::
_resp_test(EOC_db *db,EOC_msg *m,int check)
{
    if( m->type() != 15+128 ){
	return -1;
    }

    if( !db )
	return 0;

    resp_configure *resp= (resp_configure *)m->payload();
    PDEBUG(DINFO,"TEST_RESP FROM(%d), loop(%d),snr(%d)",m->src(),resp->loop_attn,resp->snr_marg);
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
    PDEBUG(DINFO,"DB: Inventory app request");
    if( !p ){
	PDEBUG(DERR,"DB Inventory: eror !p");    
	return -1;
    }
    if( db->check_exist((unit)p->unit) ){
	PDEBUG(DERR,"DB Inventory: error check exist");
	fr->negative();
	return 0;
    }
    PDEBUG(DINFO,"DB Inventory: form response");	    
    fr->response();
    PDEBUG(DINFO,"DB Inventory: prisvaivanie");
    p->eoc_softw_ver = db->units[p->unit-1]->eoc_softw_ver();
    p->inv = db->units[p->unit-1]->inventory_info();
    p->region1 = 1;
    p->region0 = 1;
    PDEBUG(DINFO,"DB Inventory: success");
    return 0;
}

int EOC_db::
_appreq_endpcur(EOC_db *db,app_frame *fr)
{
    endp_cur_payload *p = (endp_cur_payload*)fr->payload_ptr();
    counters_elem elem;
    time_t cur;
    EOC_loop *loop;

    PDEBUG(DINFO,"DB: Endpoint current app request");
    if( !p ){
	PDEBUG(DERR,"DB Endp cur: eror !p");    
	return -1;
    }
    if( !(loop = db->check_exist((unit)p->unit,(side)p->side,p->loop)) ){
	PDEBUG(DERR,"DB Endp cur: error check exist: unit(%d) side(%d) loop(%d)",
	    p->unit,p->side,p->loop);	
	fr->negative();
	return 0;
    }
    PDEBUG(DINFO,"DB Endp cur: form response");
    if( time(&cur) < 0 ){
	fr->negative();
	return 0;
    }
    
    fr->response();
    p->cur_attn = loop->cur_attn();
    p->cur_snr = loop->cur_snr();
    p->total = loop->cur_counters();
    p->cur_status = loop->cur_status();

    loop->m15_counters(0,elem);
    p->cur15min = elem.cntrs;
    p->cur_15m_elaps = cur - elem.tstamp;
/*
    PDEBUG(DINFO,"ENDP 15 MIN: cur=%d, tstamp = %d",cur,elem.tstamp);
    
    PDEBUG(DINFO,"ENDP 15 MIN: cur=%s, tstamp = %s",
	asctime(localtime(&cur)),asctime(localtime(&elem.tstamp)) );
*/
    PDEBUG(DINFO,"ENDP CUR: es=%d",p->cur15min.es );

    loop->d1_counters(0,elem);
    p->cur1day = elem.cntrs;
    p->cur_1d_elaps = cur - elem.tstamp;


    return 0;
}

int EOC_db::
_appreq_endp15min(EOC_db *db,app_frame *fr)
{
    endp_15min_payload *p = (endp_15min_payload*)fr->payload_ptr();
    EOC_loop *loop;
    counters_elem elem;

    PDEBUG(DINFO,"DB: Endpoint 15 min app request");
    if( !p ){
	PDEBUG(DERR,"DB Endp 15min: eror !p");
	return -1;
    }
    if( !(loop = db->check_exist((unit)p->unit,(side)p->side,p->loop)) ){
	PDEBUG(DERR,"DB Endp 15 min: error check exist");
	fr->negative();
	return 0;
    }
    PDEBUG(DINFO,"DB Endp 15min: form response");

    switch(fr->type()){
    case APP_GET:
	if( loop->m15_counters(p->int_num,elem) ){
	    fr->negative();
	    return 0;
	}
	p->cntrs = elem.cntrs;
	fr->response();
	return 0;
    case APP_GET_NEXT:
    {
	int int_num = p->int_num;
	if( loop->m15_nx_counters(int_num,elem) ){
	    fr->negative();
    	    return 0;
	}
	p->cntrs = elem.cntrs;
	p->int_num = int_num;
	fr->response();
        return 0;
    }
    default:
        fr->negative();
	return 0;
    }
}

int EOC_db::
_appreq_endp1day(EOC_db *db,app_frame *fr)
{
    endp_1day_payload *p = (endp_1day_payload*)fr->payload_ptr();
    EOC_loop *loop;
    counters_elem elem;

    PDEBUG(DINFO,"DB: Endpoint 1 day app request");
    if( !p ){
	PDEBUG(DERR,"DB Endp 1 day: eror !p");    
	return -1;
    }
    if( !(loop = db->check_exist((unit)p->unit,(side)p->side,p->loop)) ){
	PDEBUG(DERR,"DB Endp 1 day: error check exist");
	fr->negative();
	return 0;
    }
    PDEBUG(DINFO,"DB Endp 1 day: form response");

    switch(fr->type()){
    case APP_GET:
	if( loop->d1_counters(p->int_num,elem) ){
	    fr->negative();
	    return 0;
	}
	p->cntrs = elem.cntrs;
	fr->response();
	return 0;
    case APP_GET_NEXT:
    {
	int int_num = p->int_num;
	if( loop->d1_nx_counters(int_num,elem) ){
	    fr->negative();
    	    return 0;
	}
	p->cntrs = elem.cntrs;
	p->int_num = int_num;
	fr->response();
        return 0;
    }
    default:
        fr->negative();
	return 0;
    }
}

int EOC_db::
_appreq_endpmaint(EOC_db *db,app_frame *fr)
{
    fr->negative();
    return 0;
}

int EOC_db::
_appreq_unitmaint(EOC_db *db,app_frame *fr)
{
    fr->negative();
    return 0;
}

