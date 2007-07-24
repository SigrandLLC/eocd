#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_engine.h>
#include <handlers/EOC_poller_req.h>

int
EOC_engine::register_handlers(){
    if(poll){
	poll->register_request(REQ_DISCOVERY,_req_discovery);
	poll->register_request(REQ_INVENTORY,_req_inventory);
	poll->register_request(REQ_CONFIGURE,_req_configure);
	poll->register_request(REQ_STATUS,_req_status);
	poll->register_request(15,_req_test);
    }
}
//----------------------------------------------------------------------
// Terminal constructor
EOC_engine::EOC_engine(EOC_dev *d1,int ticks_p_min,u16 rmax)
{
    ASSERT( d1 );
    type = slave;
    recv_max = rmax;
    rtr = new EOC_router(type,d1);
    resp = new EOC_responder(rtr);
    cfg = NULL;
    poll = NULL;
}

// Terminal constructor
EOC_engine::EOC_engine(EOC_dev_master *d1,int ticks_p_min,u16 rmax)
{
    ASSERT( d1 );
    type = master;
    cfg = NULL;
    poll = NULL;
    recv_max = rmax;
    rtr = new EOC_router(type,(EOC_dev *)d1);
    resp = new EOC_responder(rtr);
    cfg = new EOC_config();     
    poll = new EOC_poller(cfg,ticks_p_min,rtr->loops());
    register_handlers();
}


//----------------------------------------------------------------------
// Repeater constructor
EOC_engine::EOC_engine(EOC_dev *d1,EOC_dev *d2,u16 rmax)
{
    ASSERT( d1 && d2 );
    poll = NULL;
    cfg = NULL;
    type = repeater;
    recv_max = rmax;
    rtr = new EOC_router(type,d1,d2);
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
    ASSERT( rtr && resp ); // error in constructor

    rtr->update_state();
        
    switch( type ){
    case repeater:
	return 0;
    case master:
	if( !(dev = rtr->nsdev()) )
	    return -1;
	break;
    case slave:
	if( !(dev = rtr->csdev()) )
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
    EOC_msg *m,**ret;
    ASSERT( rtr && resp ); // Constructor failed

    if( setup_state() )
	return -1;
    
    int i=0;
    int cnt;
    
    while( (m = rtr->receive()) && i<recv_max){
	if( m->is_request() ){
	    if( resp->request(m,ret,cnt) ){
		delete m;
		return -1;
	    }
	    if( !ret ){
	    // only one message to respond
		if( rtr->send(m) ){
		    delete m;
		    return -1;
		}
	    }else if( ret ){
	    // several messages to respond	
		for(i=0;i<cnt;i++){ 
		    if( rtr->send(ret[i]) ){
			delete[] ret;
			delete m;
			return -1;
		    }
		}
		delete[] ret;
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

