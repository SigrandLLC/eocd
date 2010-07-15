/*
 * EOC_scheduler.cpp:
 * 	EOC channel units polling schedule objecht
 * 	provides schedule for request generating and sending
 */

//#define EOC_DEBUG
#include <eoc_debug.h>
#include <engine/EOC_scheduler.h>

void
EOC_scheduler::jump_Offline()
{
    PDEBUG(DFULL,"");

    for(int i=0;i<MAX_UNITS;i++)
        statem->ustates[i] = NotPresent;
    statem->state = Offline;
    send_q->clear();
    wait_q->clear();
}

int 
EOC_scheduler::jump_Setup()
{
    PDEBUG(DFULL,"");
    statem->state = Setup;
    // Schedule Discovery request
    return send_q->add(stu_c,BCAST,REQ_DISCOVERY,ts);
    return 0;
}

int
EOC_scheduler::jump_Normal()
{
    PDEBUG(DFULL,"");
    statem->state = Normal;
	// Maybe better delete this from wait queue
	PDEBUG(DFULL,"DELETE DISCOVERY from wait queue:");
    EDEBUG(DFULL,wait_q->print());    
	wait_q->find_del(BCAST,stu_c,RESP_DISCOVERY,ts);
    PDEBUG(DFULL,"-----------------after del-------------");
    EDEBUG(DFULL,wait_q->print());    
    return 0;    
}

int
EOC_scheduler::poll_unit(int ind)
{
    if(statem->ustates[ind] != Configured )
  		return -1;
    int ret = 0;
    ret += send_q->add(stu_c,(unit)(ind+1),REQ_STATUS,ts);
    return ret;    
}

int
EOC_scheduler::response(EOC_msg *m)
{
    int ind = (int)m->src()-1;
    int i;
    int not_ready = 0;

  if( !m || (m->src() == unknown) ){
		PDEBUG(DERR,"error out#1");
		if( m ){
			PDEBUG(DERR,"m->src == unknown - %s",(m->src() == unknown) ? "YES" : "NO");
		}
		return -1;
	}
	
    PDEBUG(DFULL,"-----------------response------------------");
    EDEBUG(DFULL,wait_q->print());

  if( wait_q->find_del(m->src(),m->dst(),m->type(),ts) ){
		PDEBUG(DERR,"err out#2: src=%d, dst=%d,type=%d",m->src(),m->dst(),m->type());
		return -1;
	}

    PDEBUG(DFULL,"-----------------after find_del-------------");
    EDEBUG(DFULL,wait_q->print());    
    PDEBUG(DFULL,"-----------------end response---------------");

    PDEBUG(DFULL,"RESPONSE: src(%d),dst(%d),type(%d)",m->src(),m->dst(),m->type());
    
    switch( m->type() ){	
    case RESP_DISCOVERY:
		if( statem->state != Setup ){
			PDEBUG(DERR,"RESPONSE DROP (not Setup): src(%d),dst(%d),type(%d)",m->src(),m->dst(),m->type());
    	    return -1;
		}
		if( statem->ustates[ind] != NotPresent ){
			PDEBUG(DERR,"RESPONSE DROP (not NotPresent): src(%d),dst(%d),type(%d)",m->src(),m->dst(),m->type());
			// Not in proper state - drop
			if( statem->ustates[(int)stu_c-1] == Discovered &&
				statem->ustates[(int)stu_r-1] == Discovered ){
				PDEBUG(DERR,"All units discovered!");
				return 0;
			}
			return wait_q->add(BCAST,stu_c,RESP_DISCOVERY,ts);
			return 0;
		}
		statem->ustates[ind] = Discovered;

		if( send_q->add(stu_c,m->src(),REQ_INVENTORY,ts) )
			return -1;

		return wait_q->add(BCAST,stu_c,RESP_DISCOVERY,ts);

    case RESP_INVENTORY:
		if( statem->state != Setup ){
      PDEBUG(DERR,"statem->state != Setup, Setup=%d, statem=%d",Setup,statem->state);
			return -1;
    }
		if( statem->ustates[ind] != Discovered ){
			// Not in proper state - drop
      PDEBUG(DERR,"statem->ustates[ind] != Discovered, Discovered=%d, ustates=%d",
        Discovered,statem->ustates[ind]);
			return -1;
		}
		statem->ustates[ind] = Inventored;
		return send_q->add(stu_c,m->src(),REQ_CONFIGURE,ts);
    case RESP_CONFIGURE:
		if( statem->state != Setup )
			return -1;
		if( statem->ustates[ind] != Inventored ){
			// Not in proper state - drop
			return -1;
		}
		statem->ustates[ind] = Configured;
		poll_unit(ind);

		if( statem->ustates[(int)stu_r-1] == Configured ){
			for(i=0;statem->ustates[i] != NotPresent;i++){
				if( statem->ustates[i] !=Configured )
					not_ready = 1;
			}
			if( not_ready )
				break;
			if( !jump_Normal() )
				break;
			if( !jump_Setup() )
				break;
			jump_Offline();
		} 
		break;
    case RESP_NSIDE_PERF:
    case RESP_CSIDE_PERF:
    case RESP_MAINT_STAT:
    case RESP_SENSOR_STATE:
		// have no corresponding requests - responses to STATUS request
        break;	
    default:
        send_q->add(stu_c,m->src(),RESP2REQ(m->type()),ts+ts_offs);
        break;
    }
    return 0;
}

int
EOC_scheduler::request(sched_elem &el)
{
	int flag = 1;
	while( flag ){
		if( send_q->schedule(el,ts) ){
			return -1;	
		}

		PDEBUG(DFULL,"REQUEST: src(%d),dst(%d),type(%d)",el.src,el.dst,el.type);

		sched_elem n = el;
		unit swap = n.src;
		n.src = n.dst;
		n.dst = swap;
		n.tstamp = ts;		
		sched_elem n1 = n;
		switch( n.type ){
		case REQ_SRST_BCKOFF:
			// no response!
			break;
		case REQ_STATUS:
			// maybe 3 additional responses
			PDEBUG(DFULL,"STATUS REQUEST: ts=%d, tv=%u",ts.get_val(),time(NULL));
			n1.type = RESP_NSIDE_PERF;
			wait_q->add(n1);
			n1.type = RESP_CSIDE_PERF;
			wait_q->add(n1);
			n1.type = RESP_MAINT_STAT;
			wait_q->add(n1);
			n1.type = RESP_SENSOR_STATE;
			wait_q->add(n1);
		default:
			n.type = REQ2RESP(n.type);
			PDEBUG(DFULL,"TO_WAIT_Q: src(%d),dst(%d),type(%d)",n.src,n.dst,n.type);
			n.tstamp = ts;
			wait_q->add(n);
		}
		break;
	}
    return 0;
}

int
EOC_scheduler::resched()
{
    sched_elem el;
    int ret;
    PDEBUG(DFULL,"RESCHED");
    EDEBUG(DFULL,wait_q->print());
    while( !(ret=wait_q->get_old(ts,wait_to,el)) ){
		switch( el.type ){
		case RESP_NSIDE_PERF:
		case RESP_CSIDE_PERF:
		case RESP_MAINT_STAT:
			break;
		default:
    	    PDEBUG(DFULL,"RESCHED: src(%d) dst(%d) type(%d) ts(%d)",el.src,el.dst,el.type,el.tstamp.get_val());
			unit swap = el.src;
			el.src = el.dst;
			el.dst = swap;
			el.type += RESP_OFFSET; 
			el.tstamp = ts;
			send_q->add(el);
		}
    }
    return 0;
}
