/*
 * EOC_scheduler.cpp:
 * 	EOC channel units polling schedule objecht
 * 	provides schedule for request generating and sending
 */

#include <engine/EOC_scheduler.h>

void
EOC_scheduler::jump_Offline()
{
    for(int i=0;i<MAX_UNITS;i++)
        statem->ustates[i] = NotPresent;
    statem->state = Offline;
    send_q->clear();
    wait_q->clear();
}

int 
EOC_scheduler::jump_Setup()
{
    statem->state = Setup;
    // Schedule Discovery request
    return send_q->add(stu_c,BCAST,REQ_DISCOVERY,ts);
    return 0;
}

int
EOC_scheduler::jump_Normal()
{
    printf("Jump Normal\n");
    statem->state = Normal;
    
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

    if( !m || (m->src() == unknown) )
	return -1;
//    printf("-----------------response------------------\n");
//    wait_q->print();
    if( wait_q->find_del(m->src(),m->dst(),m->type(),ts) )
	return -1;
//    printf("-----------------after find_del-------------\n");
//    wait_q->print();    
//    printf("-----------------end response---------------\n");

    printf("RESPONSE: src(%d),dst(%d),type(%d)\n",m->src(),m->dst(),m->type());
    
    switch( m->type() ){	
    case RESP_DISCOVERY:
	if( statem->state != Setup ){
	    printf("RESPONSE DROP (not Setup): src(%d),dst(%d),type(%d)\n",m->src(),m->dst(),m->type());
    	    return -1;
	}
	if( statem->ustates[ind] != NotPresent ){
	    printf("RESPONSE DROP (not NotPresent): src(%d),dst(%d),type(%d)\n",m->src(),m->dst(),m->type());
	    // Not in proper state - drop
	    if( statem->ustates[(int)stu_c-1] == Discovered &&
		statem->ustates[(int)stu_r-1] == Discovered ){
		printf("All is discovered!\n");
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
	if( statem->state != Setup )
	    return -1;
	if( statem->ustates[ind] != Discovered ){
	    // Not in proper state - drop
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
    if( send_q->schedule(el,ts) ){
	return -1;	
    }
    if( el.type == REQ_DISCOVERY )
	printf("REQUEST: src(%d),dst(%d),type(%d)\n",el.src,el.dst,el.type);
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
	n1.type = RESP_NSIDE_PERF;
	wait_q->add(n1);
	n1.type = RESP_CSIDE_PERF;
	wait_q->add(n1);
	n1.type = RESP_MAINT_STAT;
	wait_q->add(n1);
    default:
	n.type = REQ2RESP(n.type);
//        printf("TO_WAIT_Q: src(%d),dst(%d),type(%d)\n",n.src,n.dst,n.type);
	n.tstamp = ts;
	wait_q->add(n);
    }
    return 0;
}

int
EOC_scheduler::resched()
{
    sched_elem el;
    int ret;
//    printf("RESCHED\n");
//    wait_q->print();
    while( !(ret=wait_q->get_old(ts,wait_to,el)) ){
	switch( el.type ){
	case RESP_NSIDE_PERF:
	case RESP_CSIDE_PERF:
	case RESP_MAINT_STAT:
	    break;
	default:
//    	    printf("RESCHED: src(%d) dst(%d) type(%d) ts(%d)\n",el.src,el.dst,el.type,el.tstamp.get_val());
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
