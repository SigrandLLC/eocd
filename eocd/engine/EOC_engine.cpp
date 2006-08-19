#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_engine.h>
#include <handlers/EOC_poller_req.h>
#include <eoc_debug.h>

// Terminal constructor
EOC_engine::EOC_engine(EOC_dev_terminal *d1,dev_type t,u16 rmax)
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
    static int number = -1;
    EOC_msg *m,**ret;
    ASSERT( rtr && resp ); // Constructor failed

/*
    if( setup_state() )
	return -1;
*/    
    int i=0;
    int cnt;
    number++;
    PDEBUG(DFULL,"%d schedule started\n",number);
    while( (m = rtr->receive()) && i<recv_max){
        PDEBUG(DFULL,"%d schedule: message: src(%d) dst(%d) id(%d)\n",number,m->src(),m->dst(),m->type());
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
			for(int j=0;j<cnt;j++){
			    delete ret[j];
			}
			delete[] ret;
			delete m;
			return -1;
		    }
		}
		for(int j=0;j<cnt;j++){
		    delete ret[j];
		}
		delete[] ret;
	    }
	}
	delete m;
	i++;
    }
    return 0;
}

int EOC_engine::
configure(char *ch_name)
{
    EOC_dev_terminal *dev;
    
    switch(type){	
    case master:
        PDEBUG(DERR,"(%s): Request slave configuration for master!",ch_name);
        return -1;
    case slave:
    {
        dev = (EOC_dev_terminal*)rtr->csdev();
        break;
    }
    default:
        return 0;
    }
    if( !dev ){
        PDEBUG(DERR,"(%s): Error router initialisation",ch_name);
        return -1;
    }
    dev->configure();
    return 0;
}

