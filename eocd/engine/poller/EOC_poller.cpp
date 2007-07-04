/*
 * EOC_poller.cpp:
 * 	master EOC polling class
 *	provides scheduleable polling of SHDSL channel devices
 *	and storing of SHDSL channel state & configuration DataBase
 */

#include <generic/EOC_msg.h>
#include <engine/EOC_scheduler.h>
#include <engine/EOC_db.h>
#include <engine/EOC_poller.h>


int
EOC_poller::register_request(unsigned char type,request_handler_t h)
{
    if( type >=REQUEST_QUAN || req_hndl[type] )
	return -1;
    req_hndl[type] = h;
}

int
EOC_poller::unregister_request(unsigned char  type)
{
    if( type >=REQUEST_QUAN || !req_hndl[type] )
	return -1;
    req_hndl[type] = NULL;
    return 0;
}

int
EOC_poller::register_response(int type,request_handler_t h)
{
    unsigned char ind = type-REQUEST_QUAN;
    if( ind >= RESPONSE_QUAN || resp_hndl[ind] )
	return -1;
    resp_hndl[ind] = h;
}

int
EOC_poller::unregister_response(int type)
{
    unsigned char ind = type-REQUEST_QUAN;
    if( ind >= RESPONSE_QUAN || !resp_hndl[ind] )
	return -1;
    resp_hndl[ind] = NULL;
    return 0;
}


EOC_msg *
EOC_poller::gen_request(){
    unsigned char type;
    unit dst;
    int ind;
    if( sch->request(&type,&dst) )
	return NULL;
    if( type>=REQUEST_QUAN || !req_hndl[type] ){
	PDEBUG(0,"Scheduled invalid type of message: %d",type);
	return NULL;
    }    
    return req_hndl[type]();
}

int
EOC_poller::process_msg(EOC_msg *m)
{
    // Check that we have assosiated handler 
    // & we request this response
    unsigned char type = m->type()-REQUEST_QUAN;
    if( !m->is_response() || !sch->response(m) )
	return -1;
    if( type >= RESPONSE_QUAN || !resp_hndl[type] )
	return -1;
    return resp_hndl[type](db,m);
}
