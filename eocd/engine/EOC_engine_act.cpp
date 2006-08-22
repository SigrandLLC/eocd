#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>

#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_engine_act.h>

#include <handlers/EOC_poller_req.h>

#include <conf_profile.h>
#include <shdsl/config.h>
#include <eoc_debug.h>

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
EOC_engine_act(EOC_dev_terminal *d1,EOC_config *c,u16 ticks_p_min,u16 rmax) : 
    EOC_engine(d1,master,rmax)
{
    ASSERT( d1 );
    recv_max = rmax;
    cfg=c;
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
	if( !(dev = rtr->csdev()) )
	    return -1;
	break;
    case slave:
	if( !(dev = rtr->nsdev()) )
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

int EOC_engine_act::
configure(char *ch_name)
{
    EOC_dev_terminal *dev;
    PDEBUG(DINFO,"start");
    switch(type){
    case master:
        dev = (EOC_dev_terminal*)rtr->nsdev();
        break;
    case slave:
        dev = (EOC_dev_terminal*)rtr->csdev();
        break;
    default:
        return 0;
    }
    if( !dev ){
        PDEBUG(DERR,"(%s): Error router initialisation",ch_name);
        return -1;
    }
    conf_profile *prof = (conf_profile *)
	    cfg->conf_tbl()->find((char*)cfg->conf_prof_name(),strlen(cfg->conf_prof_name()) );
    if( !prof ){
        PDEBUG(DERR,"Cannot find corresponding profile");
	return -1;
    }	
    return dev->configure(prof->conf);
}

int EOC_engine_act::
app_request(app_frame *fr){ 

    switch( fr->id() ){
    case APP_SPAN_PARAMS:
    {
	span_params_payload *p = (span_params_payload *)fr->payload_ptr();
	p->units = poll->unit_quan();
	p->loops = rtr->loops();
	p->link_establ = poll->link_established();
	return 0;
    }
    default:
        return poll->app_request(fr);
    }
}
