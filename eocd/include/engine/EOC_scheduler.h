/*
 * EOC_scheduler.h:
 * 	EOC channel units polling schedule objecht
 * 	provides schedule for request generating and sending
 */

#ifndef EOC_SCHEDULER_H
#define EOC_SCHEDULER_H

#include <generic/EOC_msg.h>
#include <engine/sched_queue.h>


class EOC_scheduler{
protected:
    enum unit_state{ NotPresent,Discovered, Inventored, Cpnfigured};
    enum chan_state{ Offline,Discovery,Online };
    struct state_machine{
	// TODO: Maybe linked list ??
	unit_state ustates[EOC_MAX_UNITS];
	chan_state state;
    } *statem;
    __timestamp ts;
    // Task queues
    sched_queue *send_q,*wait_q;
    // State change functions
    void jump_Offline();
    int jump_Discovery();
    int jump_Online();

public:
    EOC_scheduler(){
	send_q = new sched_queue;
	wait_q = new sched_queue;
	statem = new state_machine;
	jump_Offline();
    }
    //
    inline int link_state(EOC_dev::Linkstate st){
	switch(st){
	case EOC_dev::OFFLINE:
	    jump_Offline();
	case EOC_dev::ONLINE:
	    if( jump_Discovery() )
		jump_Offline();
	}
    }
    inline void sched_tick(){ ts++; }
    // Schedule request & check response to me scheduled
    int request(unit &src,unit &dst,unsigned char &type);
    int response(EOC_msg *m);
    // State changing
    int jump_Online();
    void jump_Offline();
    int jump_Discovery();
    
};

void
EOC_scheduler::jump_Offline()
{
    for(int i=0;i<EOC_MAX_UNITS;i++)
        ustates[i] = NotPresent;
    state = Offline;
    send_q->clear();
    wait_q->clear();
}

int 
EOC_scheduler::jump_Discovery()
{
    statem->state = Discovery;
    // Schedule Discovery request
    return send_q->add(src,dst,REQ_DISCOVERY,IMMEDIATELY);
    return 0;
}

int
EOC_scheduler::jump_Online()
{
    statem->state = Online;
    EOC_msg *m ;
    // Add to send queue all periodic messages (as status requests and other)    
    return send_q->add(src,dst,2,IMMEDIATELY);    
}

int
EOC_scheduler::response(EOC_msg *m)
{
    int ind = (int)m->src()-1;
    int i;

    if( !m )
	return -1;

    if( wait_q->find_del(m->dst(),m->src(),m->type()) )
	return -1;
    
    switch( m->type() ){	
    case RESP_DISCOVERY:
	if( statem->state != Discovery )
	    return -1;
	if( statem->ustates[ind] != NotPresent ){
	    // Not in proper state - drop
	    return -1;
	}
	statem->ustates[ind] = Discovered;
	return send_q->add(stu_c,m->src(),REQ_INVENTORY,IMMEDIATELY);
    case RESP_INVENTORY:
	if( statem->state != Discovery )
	    return -1;
	if( statem->ustates[ind] != Discovered ){
	    // Not in proper state - drop
	    return -1;
	}
	statem->ustates[ind] = Inventored;
	return send_q->add(stu_c,m->src(),REQ_CONFIGURE,IMMEDIATELY);
    case RESP_CONFIGURE:
	if( statem->state != Discovery )
	    return -1;
	if( statem->ustates[ind] != Inventored ){
	    // Not in proper state - drop
	    return -1;
	}
	statem->ustates[ind] = Configured;
	int not_ready = 0;
	if( statem->ustates[(int)stu_r-1] == Configured ){
	    for(i=0;statem->ustates[i] != NotPresent;i++){
		if( statem->ustates[i] !=Configured )
		    not_ready = 1;
	    }
	    if( not_ready )
		break;
	    if( !jump_Online() )
		break;
	    if( !jump_Discovery() )
		break;
	    jump_Offline();
	} 
	break;
    }
}

int
EOC_scheduler::request(unit &src,unit &dst,unsigned char &type)
{
    if( send_q.schedule(src,dst,type,ts,wait_q) ){
	return -1;	
    } 
    return 0;
}


/*
Очередь запросов, на которые не поступило ответов. Описывается:
1. Тип ожидаемого ответа
2. Время отсылки запроса
3. Источник ответа (фиксированный или широковещательный)
4. Перевод в новое состояние ???? Кого? 
    (Discovery -ответ переводитконкретный элемент в состояние Inventory)
    4 состояния - 1. Discovery, 2. Inventory, 3. Configure 4. Online
5. 

Машина состояний для канала:
1. Offline - нет связи
2. Discovery:
    1.1 Отослать запрос
    1.2 Собирать ответы пока не придет ответ от слейва, каждый новый эл-т канала
        добавляется в БД и переходит в состояние Inventory
3. Online - если каждый из юнитов успешно прошел все стадии 
	( 1. Discovery, 2. Inventory, 3. Configure )

Для юнита:
1. Discovered - элемент создан
2. Inventored - пришел Inventory response
3. Configured - пришел Configure response
    
*/
#endif
