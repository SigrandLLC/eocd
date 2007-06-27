#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <devs/EOC_dev.h>
#include <engine/EOC_router.h>

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
	break;
    case slave:
        ifs[if_cnt].sunit = stu_r;
	ifs[if_cnt].in_dir = EOC_msg::DOWNSTREAM;
	ifs[if_cnt].out_dir = EOC_msg::UPSTREAM;	
	break;
    default:
	return;
    }
    ifs[if_cnt].sdev = side;
    if_cnt++;
    type = r;
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
    ifs[if_cnt++].sdev = nside;
    ifs[if_cnt].in_dir = EOC_msg::DOWNSTREAM;
    ifs[if_cnt].out_dir = EOC_msg::UPSTREAM;    
    ifs[if_cnt++].sdev = cside;
    type = r;
}

EOC_router::~EOC_router(){
    int i;
    for(i=0;i<if_cnt;i++){
	delete ifs[i].sdev;
    }
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
    EOC_msg *m,*ret = NULL;
    EOC_dev *poll_dev = ifs[if_poll].sdev;
    EOC_dev *route_dev = get_route_dev();
    EOC_msg::Direction dir = ifs[if_poll].in_dir;
    unit u = ifs[if_poll].sunit;
    

    while( (m = poll_dev->recv()) && (icnt<max_recv_msg) ){
	m->direction(dir);
	if( u == unknown ){
	    if( m->type() == DISCOVERY ){
		ret = m;
		goto exit;
	    }else{
		delete m;
		icnt++;
	        continue;		    
	    }
	}

	if( m->dst() == u || m->dst() == BCAST ){
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
    if( m->direction() == EOC_msg::UNDEFINED ){
	if( out_direction(&dir) ){
	    return -1;
	}
	m->direction(dir);
    }
    dir = m->direction();
    for(i=0;i<if_cnt;i++){
	if( ifs[i].out_dir == dir ){
	    return ifs[i].sdev->send(m);
	}
    }
    return -1;
}
