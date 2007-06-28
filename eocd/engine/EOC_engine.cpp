#include <engine/EOC_msg.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>

EOC_engine::EOC_engine(dev_type r,EOC_dev *d1,EOC_dev *d2)
{
    type = r;
    rtr = new EOC_router(r,d1,d2);
    resp = new EOC_responder(rtr);
    if( r == master ){
        poll = new EOC_poller;
    }
}

int
EOC_engine::schedule()
{
    EOC_msg *m;
    // Receive one EOC message
    setup_state();

    if( state == EOC_OFFLINE )
	return 0;
	
    if( m = rtr->receive() ){
	if( m->is_request() ){
	    m = resp->process(m);
	    if( rtr->send(m) ){
		delete m;
		return -1;
	    }
	}else if( m->is_response() ){
	    if( type == master ){
		if( poll->recv_resp(m) ){
		    delete m;
		    return -2;
		}
	}
	delete m;
    }

    // Send one EOC request
    if( type == master ){
	if( m = poll->gen_req() ){
	    if( rtr->send(m) ){
		delete m;
		return -3;
	    }
	    delete m;
        }
    }

    return 0;
}

