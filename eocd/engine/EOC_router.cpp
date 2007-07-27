#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <devs/EOC_dev.h>
#include <engine/EOC_router.h>
#include <generic/EOC_responses.h>
#include <generic/EOC_requests.h>
#include <eoc_debug.h>

EOC_router::EOC_router(dev_type r,EOC_dev *side)
{
    int i;
    // initial initialising
    if( !side ) 
	return;
    zero_init();
    // setup router
    switch(r){
    case master:	
        ifs[if_cnt].sunit = stu_c;
	ifs[if_cnt].out_dir = EOC_msg::DOWNSTREAM;
	ifs[if_cnt].in_dir = EOC_msg::UPSTREAM;
	ifs[if_cnt].state = eoc_Offline;
	break;
    case slave:
        ifs[if_cnt].sunit = stu_r;
	ifs[if_cnt].in_dir = EOC_msg::DOWNSTREAM;
	ifs[if_cnt].out_dir = EOC_msg::UPSTREAM;
	ifs[if_cnt].state = eoc_Offline;
	break;
    default:
	return;
    }
    ifs[if_cnt].sdev = side;
    if_cnt++;
    type = r;
    // loopback setup
    loop_head = loop_tail = 0;
}
	
EOC_router::EOC_router(dev_type r,EOC_dev *nside,EOC_dev *cside)
{
    int i;
    // initial initialising
    zero_init();
    // setup router

    if( r != repeater || !nside || !cside)
	return;

    ifs[if_cnt].in_dir = EOC_msg::UPSTREAM;
    ifs[if_cnt].out_dir = EOC_msg::DOWNSTREAM;    
    ifs[if_cnt].state = eoc_Offline;
    ifs[if_cnt++].sdev = nside;
    ifs[if_cnt].in_dir = EOC_msg::DOWNSTREAM;
    ifs[if_cnt].out_dir = EOC_msg::UPSTREAM;    
    ifs[if_cnt].state = eoc_Offline;
    ifs[if_cnt++].sdev = cside;
    type = r;
    update_state();
    // loopback setup
    loop_head = loop_tail = 0;

}

EOC_router::~EOC_router(){
    int i;
    for(i=0;i<if_cnt;i++){
	delete ifs[i].sdev;
    }
}

inline void
EOC_router::zero_init(){
    int i;
    // initial initialising
    for(i=0;i<SHDSL_MAX_IF;i++){
        ifs[i].sdev = NULL;
        ifs[i].sunit = unknown;
	ifs[i].in_dir = ifs[i].out_dir = EOC_msg::NOSTREAM;
    }
    if_cnt = 0;
}

inline EOC_dev *
EOC_router::get_route_dev(int if_ind){
    max_recv_msg = 5;
    if( type != repeater || !if_cnt )
        return NULL;
    int ind = (if_ind+1)<if_cnt ? if_ind+1 : 0;
    return ifs[ind].sdev;
}

inline int
EOC_router::out_direction(EOC_msg::Direction *dir){
    if( !if_cnt || (type==repeater) )
	return -1;
    switch(type){
    case master:
	*dir = EOC_msg::DOWNSTREAM;
	return 0;
    case slave:
	*dir = EOC_msg::UPSTREAM;
	return 0;
    }
    return -1;
}

inline void
EOC_router::update_state()
{
    struct interface *iface;
    // get link status from device
    for(int i=0;i<if_cnt;i++){
	iface = &ifs[i];
	EOC_dev::Linkstate link = iface->sdev->link_state();
	// check physical link
	if( link == EOC_dev::OFFLINE ){
	    iface->state = eoc_Offline;
	    iface->sunit = unknown;
	    continue;
	}
	// check for discovery phase comlete
	if( iface->sunit == unknown ){
	    iface->state = eoc_Discovery;
	    continue;
	}
	// online state
	iface->state = eoc_Online;
    }
}

inline EOC_msg*
EOC_router::process_discovery(int if_ind,EOC_msg *m)
{
    EOC_msg *resp;
    EOC_dev *dev = get_route_dev(if_ind);
    unit u;
    resp_discovery *r;
    
    (*m->payload())++;    
    u = (unit)(*m->payload() + 2);
    resp = new EOC_msg(m,RESP_DISCOVERY_SZ);

    if( !( u >=sru1 && u <= sru10) && type == repeater )
	return NULL;
    // setup unit address information
    switch( type ){
    case repeater:
	ifs[if_ind].sunit = u;
    default:
	// increment Hop count and send
        resp->response(RESP_DISCOVERY_SZ);
	resp->src(ifs[if_ind].sunit);
        r=(resp_discovery*)resp->payload();
	// Setup some fields	
	strcpy((char*)r->vendor_id,"Sigrand");
        r->eoc_softw_ver=1;
	r->shdsl_ver = 0x80;
	if( dev )
	    r->fwd_loss = (dev->link_state()==EOC_dev::OFFLINE) ? 1 : 0;
    }
    return resp;
}

unit
EOC_router::csunit()
{
    int i;
    if( !if_cnt )
	return err;
	
    switch( type ){
    case slave:
	return err;
    case master:
	return stu_c;
    case repeater:
	return ifs[CS_IND].sunit;
    }
    return err;
}

unit
EOC_router::nsunit()
{
    int i;
    if( !if_cnt )
	return err;
	
    switch( type ){
    case slave:
	return stu_r;
    case master:
	return err;
    case repeater:
	return ifs[NS_IND].sunit;
    }
    return err;
}

EOC_dev *
EOC_router::csdev()
{
    int i;
    if( !if_cnt )
	return NULL;
	
    switch( type ){
    case master:
	return NULL;
    case slave:
	return ifs[0].sdev;
    case repeater:
	return ifs[CS_IND].sdev;
    }
    return NULL;
}


EOC_dev *
EOC_router::nsdev()    
{
    int i;
    if( !if_cnt )
	return NULL;
	
    switch( type ){
    case master:
	return ifs[0].sdev;
    case slave:
	return NULL;
    case repeater:
	return ifs[NS_IND].sdev;
    }
    return NULL;
}


int
EOC_router::csunit(unit u)
{
    if( !if_cnt || type == master || type == slave )
	return -1;
    ifs[CS_IND].sunit = u;
    return 0;
}

int
EOC_router::nsunit(unit u)
{
    if( !if_cnt || type == master || type == slave )
	return -1;
    ifs[NS_IND].sunit = u;
    return 0;
}

int
EOC_router::loops(){
    int nsloops = ( nsdev() ) ? nsdev()->loops() : 0;
    int csloops = ( csdev() ) ? csdev()->loops() : 0;
    if( nsloops && csloops ){
	ASSERT( nsloops == csloops );
	return csloops;
    }else if( nsloops )
	return nsloops;
    else
	return csloops;
}

int
EOC_router::term_unit(unit u)
{
    if( !if_cnt || type == repeater )
	return -1;
    ifs[0].sunit = u;
}

EOC_msg *
EOC_router::receive()
{
    int icnt=0;
    EOC_msg *m,*ret=NULL,*resp = NULL;
    EOC_dev *poll_dev = ifs[if_poll].sdev;
    EOC_dev *route_dev = get_route_dev(if_poll);
    EOC_msg::Direction dir = ifs[if_poll].in_dir;
    unit u = ifs[if_poll].sunit;

    // Side status update
    update_state();
    if( ifs[if_poll].state == eoc_Offline )
	return NULL;

    if( m = get_loop() ){
	if( (m->type() == REQ_DISCOVERY) ){
	    if( resp = process_discovery(if_poll,m) ){
		if( add_loop(resp) ){
		    /* TODO: LOG error */
		}
	    } else{
		/* TODO: LOG error */
	    }
	}else
	    return m;
    }
    
    while( (m = poll_dev->recv()) && (icnt<max_recv_msg) ){
	m->direction(dir);
	
	if( (m->type() == REQ_DISCOVERY) ){
	    if( resp = process_discovery(if_poll,m) ){
		if( route_dev )
		    route_dev->send(m);
		poll_dev->send(resp);
	    }
	    icnt++;
	    delete m;
	    continue;
	}

	// retranslate Broadcast to next device	
	if( m->dst() == BCAST && route_dev ){
	    if( route_dev->send(m) ){
		// send error handling 
	    }
	}

	if( m->dst() == u || m->dst() == BCAST ){
	    if( ifs[if_poll].state == eoc_Discovery ){
		delete m;
		icnt++;
	        continue;		    
	    }
	    ret = m;
	    goto exit;
	} else if( route_dev ){
	    if( route_dev->send(m) ){
		delete m;
		break;
	    }
	}
	delete m;
	icnt++;
    }
exit:
    if_poll = (if_poll+1)<if_cnt ?  if_poll+1 : 0;
    return ret;
}


int
EOC_router::send(EOC_msg *m)
{
    EOC_msg::Direction dir;
    int i; 
    int ret = 0;   
    u8 loop_added = 0;

    if( m->direction() == EOC_msg::NOSTREAM ){
	if( out_direction(&dir) ){
	    return -1;
	}
	m->direction(dir);
    }
    dir = m->direction();
    for(i=0;i<if_cnt;i++){
	if( (ifs[i].sunit == m->dst() || m->dst() == BCAST) && !loop_added ){
	// loopback
	    EOC_msg *new_m = new EOC_msg(m);
	    new_m->dst(ifs[i].sunit);
	    ret += add_loop(new_m);
	    loop_added = 1;
	}
	if( ifs[i].out_dir == dir ){
		ret += ifs[i].sdev->send(m);
		return ret;
	}
    }
    return ret;
}
