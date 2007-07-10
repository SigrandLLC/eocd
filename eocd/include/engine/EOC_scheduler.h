/*
 * EOC_scheduler.h:
 * 	EOC channel units polling schedule objecht
 * 	provides schedule for request generating and sending
 */

#ifndef EOC_SCHEDULER_H
#define EOC_SCHEDULER_H

#include <devs/EOC_dev.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/sched_queue.h>


class EOC_scheduler{
public:
    enum unit_state{ NotPresent,Discovered, Inventored, Configured };
    enum sched_state{ Offline,Setup,Normal };    
protected:
    struct state_machine{
	// TODO: Maybe linked list ??
	unit_state ustates[EOC_MAX_UNITS];
	sched_state state;
    } *statem;
    __timestamp ts;
    int ts_offs;
    // Task queues
    sched_queue *send_q,*wait_q;
    // State change functions
    void jump_Offline();
    int jump_Setup();
    int jump_Normal();

public:
    EOC_scheduler(unsigned int toffs){
    	send_q = new sched_queue;
	wait_q = new sched_queue;
	statem = new state_machine;
	ts_offs = toffs;
	jump_Offline();
    }
    //
    inline void link_state(EOC_dev::Linkstate st){
	switch(st){
	case EOC_dev::OFFLINE:
	    jump_Offline();
	case EOC_dev::ONLINE:
	    if( jump_Setup() )
		jump_Offline();
	}
    }
    
    sched_state state(){ return statem->state; }
    inline void tick(){ ts++; }
    
    // Schedule request & check response to me scheduled
    int request(sched_elem &el);
    int response(EOC_msg *m);
    // debug
    void print(){
    	printf("______________________________________\n");   
	printf("send_q:\n");
	send_q->print();
	printf("______________________________________\n");
	printf("wait_q:\n");
	wait_q->print();
    }
};

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
