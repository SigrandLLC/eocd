/*
 * EOC_responder.h:
 *	Defines universal EOC code part which runs on ewery unit in
 *	SHDSL span.
 *	It provides request processing.
*/


#ifndef SIGRAND_EOC_RESPONDER_H
#define SIGRAND_EOC_RESPONDER_H

#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <engine/EOC_router.h>
#include <eoc_debug.h>

class EOC_responder{
public:
typedef int (*responder_handler_t)(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt);
protected:
    responder_handler_t handlers[REQUEST_QUAN];
    EOC_router *r;
public:
    EOC_responder(EOC_router *r_in){
	r = r_in;
	for(int i=0;i<REQUEST_QUAN;i++)
	    handlers[i] = NULL;

	handlers[REQ_INVENTORY] = _inventory;
	handlers[REQ_CONFIGURE] = _configure;
	handlers[REQ_STATUS] =_status;
	handlers[15] = _test;
    }

    ~EOC_responder(){}

    int register_hndl(u8 type,responder_handler_t h){
	if( !(type < REQUEST_QUAN) || handlers[type])
	    return -1;
	handlers[type] = h;
	return 0;
    }

    int unregister_hndl(u8 type,responder_handler_t h){
	if( !(type < REQUEST_QUAN) || !handlers[type])
	    return -1;
	handlers[type] = NULL;
	return 0;
    }

    int request(EOC_msg *m,EOC_msg **&ret,int &cnt){
	if( !m->is_request() || !handlers[m->type()])
	    return -1;
	return handlers[m->type()](this,m,ret,cnt);
    }
    
    // Handlers
    static int _inventory(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt);
    static int _configure(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt);
    static int _status(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt);
    static int _test(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt);
};

#endif
