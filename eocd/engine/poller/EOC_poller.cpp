/*
 * EOC_poller.cpp:
 * 	master EOC polling class
 *	provides scheduleable polling of SHDSL channel devices
 *	and storing of SHDSL channel state & configuration DataBase
 */

#include <generic/EOC_types.h>
#include <generic/EOC_msg.h>
#include <engine/EOC_scheduler.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_handlers.h>
#include <db/EOC_db.h>

int
EOC_poller::register_request(u8 type,request_handler_t h)
{
    if( type >=REQUEST_QUAN || req_hndl[type] )
	return -1;
    req_hndl[type] = h;
}

int
EOC_poller::unregister_request(u8 type)
{
    if( type >=REQUEST_QUAN || !req_hndl[type] )
	return -1;
    req_hndl[type] = NULL;
    return 0;
}

int
EOC_poller::register_response(u8 type,response_handler_t h)
{
    unsigned char ind = type-REQUEST_QUAN;
    if( ind >= RESPONSE_QUAN || resp_hndl[ind] )
	return -1;
    resp_hndl[ind] = h;
}

int
EOC_poller::unregister_response(u8 type)
{
    unsigned char ind = type-REQUEST_QUAN;
    if( ind >= RESPONSE_QUAN || !resp_hndl[ind] )
	return -1;
    resp_hndl[ind] = NULL;
    return 0;
}


EOC_msg *
EOC_poller::gen_request(){
    sched_elem el;
    int ind;
    // get request
    sch->resched();
//    sch->print();
    if( sch->request(el) ){
	sch->tick();
	return NULL;
    }
    if( el.type>=REQUEST_QUAN || !req_hndl[el.type] ){
	//PDEBUG(0,"Scheduled invalid type of message: %d",el.type);
	return NULL;
    }
    return req_hndl[el.type](sch->state(),el,cfg);
}

int
EOC_poller::process_msg(EOC_msg *m)
{
    // Check that we have assosiated handler 
    // & we request this response
    if( !m )
	return -1;
    unsigned char type = m->type()-REQUEST_QUAN;
    if(	!m->is_response() ||		// check that it is response
	    resp_hndl[type](NULL,m) ||	// Check that response has right format 
	    sch->response(m) ||		// check that we request this response
	    type >= RESPONSE_QUAN || !resp_hndl[type] ) // check we hawe handler
	return -1;
    // commit changes to EOC DataBase
    return resp_hndl[type](db,m);
}
