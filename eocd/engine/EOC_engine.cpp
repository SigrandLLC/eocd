#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_engine.h>
#include <handlers/EOC_poller_req.h>
#include <handlers/EOC_poller_resp.h>
#include <handlers/EOC_resp_handlers.h>

int
EOC_engine::register_handlers(){
    if(poll){
	poll->register_request(REQ_DISCOVERY,_req_discovery);
	poll->register_request(REQ_INVENTORY,_req_inventory);
	poll->register_request(REQ_CONFIGURE,_req_configure);
	poll->register_request(15,_req_test);

	poll->register_response(RESP_DISCOVERY,_resp_discovery);
	poll->register_response(RESP_INVENTORY,_resp_inventory);
	poll->register_response(RESP_CONFIGURE,_resp_configure);
	poll->register_response(15+128,_resp_test);

    }
    if( resp ){
	resp->register_hndl(REQ_INVENTORY,_inventory);
	resp->register_hndl(REQ_CONFIGURE,_configure);
	resp->register_hndl(15,_test);
    }
}
//----------------------------------------------------------------------
// Terminal constructor
EOC_engine::EOC_engine(dev_type r,EOC_dev *d1,char *config,u16 rmax)
{
    cfg = NULL;
    poll = NULL;

    if( !(r == master || r == slave) ){ 
	// If this is repeater - error (need 2 devices)
	rtr = NULL;
	resp = NULL;
	return;
    }

    type = r;
    recv_max = rmax;
    rtr = new EOC_router(r,d1);
    resp = new EOC_responder(rtr);
    if( r == master ){
        cfg = new EOC_config(config);     
        poll = new EOC_poller(cfg);
    }
    register_handlers();
}

//----------------------------------------------------------------------
// Repeater constructor
EOC_engine::EOC_engine(dev_type r,EOC_dev *d1,EOC_dev *d2,u16 rmax)
{
    poll = NULL;
    cfg = NULL;

    if( !(r == repeater) ){
	rtr = NULL;
	resp = NULL;
	poll = NULL;
    } 
    type = r;
    recv_max = rmax;
    rtr = new EOC_router(r,d1,d2);
    resp = new EOC_responder(rtr);
    register_handlers();
}

//----------------------------------------------------------------------
// setup_state - setups poller link status if i really changes 
// on device

int
EOC_engine::setup_state()
{
    EOC_dev *dev;
    if( !rtr || !resp ) // error in constructor
	return -1;

    rtr->update_state();
        
    switch( type ){
    case repeater:
	return 0;
    case master:
	if( !(dev = rtr->csdev()) )
	    return -1;
	break;
    case slave:
	if( !(dev = rtr->nsdev()) )
	    return -1;
	break;
    }
    
    if( !poll )
	return 0;
	
    if( dev->link_state() == EOC_dev::OFFLINE ){
	if( poll->link_state() == EOC_dev::ONLINE )
	    poll->link_state(EOC_dev::OFFLINE);
    } else if( poll->link_state() == EOC_dev::OFFLINE ){
	poll->link_state(EOC_dev::ONLINE);
    }
    return 0;
}



//----------------------------------------------------------------------
// schedule - call it to take control to engine to process incoming and 
// outgoing messages
int
EOC_engine::schedule()
{
    EOC_msg *m;
    if( !rtr || !resp ) // Constructor failed
	return -1;
    // Receive one EOC message
    if( setup_state() )
	return -1;
    
    int i=0;
    while( (m = rtr->receive()) && i<recv_max){
	if( m->is_request() ){
	    if( resp->request(m) || rtr->send(m) ){
		delete m;
		return -1;
	    }
	}else if( m->is_response() ){
	    if( poll && poll->process_msg(m) ){
		delete m;
		return -2;
	    }
	}
	delete m;
	i++;
    }

    // Send one EOC request
    if( poll ){
	while( m = poll->gen_request() ){
	    if( rtr->send(m) ){
		delete m;
		return -3;
	    }
	    delete m;
        }
    }
    return 0;
}

