#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_engine.h>
#include <handlers/EOC_poller_req.h>

//----------------------------------------------------------------------
// Terminal constructor
EOC_engine::EOC_engine(EOC_dev *d1,u16 rmax)
{
    ASSERT( d1 );
    type = slave;
    recv_max = rmax;
    rtr = new EOC_router(type,d1);
    resp = new EOC_responder(rtr);
}

// Terminal constructor
EOC_engine::EOC_engine(dev_type t,EOC_dev *d1,u16 rmax)
{
    ASSERT( d1 && (t == master || t == slave) );
    type = t;
    recv_max = rmax;
    rtr = new EOC_router(type,d1);
    resp = new EOC_responder(rtr);
}

//----------------------------------------------------------------------
// Repeater constructor
EOC_engine::EOC_engine(EOC_dev *d1,EOC_dev *d2,u16 rmax)
{
    ASSERT( d1 && d2 );
    type = repeater;
    recv_max = rmax;
    rtr = new EOC_router(type,d1,d2);
    resp = new EOC_responder(rtr);
}

//----------------------------------------------------------------------
// setup_state - setups poller link status if i really changes 
// on device

int
EOC_engine::setup_state()
{
    ASSERT( rtr && resp ); // error in constructor
    rtr->update_state();
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
	}
	delete m;
	i++;
    }
    return 0;
}

