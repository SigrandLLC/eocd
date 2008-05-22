/*
 * EOC_poller.cpp:
 * 	master EOC polling class
 *	provides scheduleable polling of SHDSL channel devices
 *	and storing of SHDSL channel state & configuration DataBase
 */

#define EOC_DEBUG
#include<eoc_debug.h>

#include <generic/EOC_types.h>
#include <generic/EOC_msg.h>
#include <engine/EOC_scheduler.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_handlers.h>
#include <app-if/err_codes.h>
#include <db/EOC_db.h>


int EOC_poller::
register_request(u8 type,request_handler_t h)
{
    if( type >=REQUEST_QUAN || req_hndl[type] )
		return -1;
    req_hndl[type] = h;
}

int EOC_poller::
unregister_request(u8 type)
{
    if( type >=REQUEST_QUAN || !req_hndl[type] )
		return -1;
    req_hndl[type] = NULL;
    return 0;
}

EOC_msg * EOC_poller::
gen_request(){
    sched_elem el;
    int ind;
    // Refresh schedule plan
    sch->resched();
    // get request
    if( sch->request(el) ){
		return NULL;
    }
	
    if( el.type>=REQUEST_QUAN || !req_hndl[el.type] ){
		//PDEBUG(0,"Scheduled invalid type of message: %d",el.type);
		return NULL;
    }
    return req_hndl[el.type](sch->state(),el,cfg);
}

int EOC_poller::
process_msg(EOC_msg *m)
{
    // Check that we have assosiated handler 
    // & we request this response
    if( !m ){
		PDEBUG(DERR,"!m");
		return -1;
	}
    unsigned char type = m->type()-REQUEST_QUAN;
    if(	!m->is_response() ){
		PDEBUG(DERR,"!m->is_response");
		return -1;
	}
	if( db->response(m,1) ){
		PDEBUG(DERR,"db->response(m,1)");
		return -1;
	}
	if( sch->response(m) ){
		PDEBUG(DERR,"sch->response(m)");
		return -1;
	}

    // commit changes to EOC DataBase
    return db->response(m,0);
}

int EOC_poller::
app_request(app_frame *fr)
{
    switch(fr->id()){
		// 	case APP_SPAN_STATUS:{
		// 		span_status_payload *p = (span_status_payload*)fr->payload_ptr();
		// 		p->nreps = db->reg_quan();
		// 		// TODO get thisinfo from device
		// 		p->max_lrate = 0;
		// 		p->act_lrate = 0;
		// 		p->region0 = 1;
		// 		p->region1 = 1;
		// 		p->max_prate = 0;
		// 		p->act_prate = 0; 
		// 		fr->response();
		// 		break;
		// 	}
	case APP_ENDP_CONF:{
		endp_conf_payload *p = (endp_conf_payload*)fr->payload_ptr();
		if( db->check_exist((unit)p->unit,(side)p->side,p->loop) ){
			fr->negative(ERNOELEM);
			return 0;
		}
		strncpy(p->alarm_prof,cfg->aprof(),SNMP_ADMIN_LEN);
		break;
	}
	default:
		PDEBUG(DERR,"DB request: %p",db);
		return db->app_request(fr);
	}
	return 0;
}

void EOC_poller::
link_state(EOC_dev::Linkstate link)
{
	sch->link_state(link);
	switch(link){
	case EOC_dev::OFFLINE:
		db->link_down();
		break;
	case EOC_dev::ONLINE:
		db->link_up();
		break;
	}
}
