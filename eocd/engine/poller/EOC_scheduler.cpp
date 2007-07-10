/*
 * EOC_scheduler.cpp:
 * 	EOC channel units polling schedule objecht
 * 	provides schedule for request generating and sending
 */

#include <engine/EOC_scheduler.h>

void
EOC_scheduler::jump_Offline()
{
    for(int i=0;i<EOC_MAX_UNITS;i++)
        statem->ustates[i] = NotPresent;
    statem->state = Offline;
    send_q->clear();
    wait_q->clear();
}

int 
EOC_scheduler::jump_Setup()
{
    statem->state = Setup;
    statem->ustates[0] = Configured; // This is stu_c - no need in remote configuration 
    // Schedule Discovery request
    return send_q->add(stu_c,BCAST,REQ_DISCOVERY,ts);
    return 0;
}

int
EOC_scheduler::jump_Normal()
{
    statem->state = Normal;
    // Add to send queue all periodic messages (as status requests and other)    
    int i=(int)stu_c;
    while(statem->ustates[i] != NotPresent ){
	if( send_q->add(stu_c,(unit)(i+1),15,ts) )
	    return -1;
	i++;
    }
    return 0;    
}

int
EOC_scheduler::response(EOC_msg *m)
{
    int ind = (int)m->src()-1;
    int i;
    int not_ready = 0;

    if( !m )
	return -1;

    if( wait_q->find_del(m->src(),m->dst(),m->type()) )
	return -1;

    
    switch( m->type() ){	
    case RESP_DISCOVERY:
	if( statem->state != Setup )
	    return -1;
	if( statem->ustates[ind] != NotPresent ){
	    // Not in proper state - drop
	    return -1;
	}
	statem->ustates[ind] = Discovered;
	if( send_q->add(stu_c,m->src(),REQ_INVENTORY,ts) )
	    return -1;
	if( m->src() != stu_r )
	    return wait_q->add(BCAST,stu_c,RESP_DISCOVERY,ts);
	return 0;
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
    default:
	send_q->add(stu_c,m->src(),m->type()-RESP_OFFSET,ts+ts_offs);
	break;
    }
    return 0;
}

int
EOC_scheduler::request(sched_elem &el)
{
    if( send_q->schedule(el,ts,*wait_q) ){
	return -1;	
    } 
    return 0;
}
