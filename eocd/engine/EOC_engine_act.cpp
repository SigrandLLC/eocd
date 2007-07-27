#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>

#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_engine_act.h>

#include <handlers/EOC_poller_req.h>

int EOC_engine_act::
register_handlers(){
    if(poll){
	poll->register_request(REQ_DISCOVERY,_req_discovery);
	poll->register_request(REQ_INVENTORY,_req_inventory);
	poll->register_request(REQ_CONFIGURE,_req_configure);
	poll->register_request(REQ_STATUS,_req_status);
	poll->register_request(15,_req_test);
    }
}

//--------------------------------------------------------------------------------------
// Terminal constructor

EOC_engine_act::
EOC_engine_act(EOC_dev_master *d1,EOC_config *cfg,u16 ticks_p_min,u16 rmax) : 
    EOC_engine(master,(EOC_dev*)d1,rmax)
{
    ASSERT( d1 );
    recv_max = rmax;
    poll = new EOC_poller(cfg,ticks_p_min,rtr->loops());
    register_handlers();
}

//----------------------------------------------------------------------
// setup_state - setups poller link status if i really changes 
// on device

int EOC_engine_act::
setup_state_act()
{
    EOC_dev *dev;

    if( setup_state() )
	return -1;

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
int EOC_engine_act::
schedule()
{
    EOC_msg *m,**ret;
    ASSERT( rtr && resp ); // Constructor failed

    if( setup_state_act() )
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
	    if( poll->process_msg(m) ){
		delete m;
		return -2;
	    }
	}
	delete m;
	i++;
    }

    // Send one EOC request
    while( m = poll->gen_request() ){
	if( rtr->send(m) ){
	    delete m;
	    return -3;
	}
	delete m;
    }
    return 0;
}

