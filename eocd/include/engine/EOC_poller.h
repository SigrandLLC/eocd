/*
 * EOC_poller.h:
 * 	master EOC polling class
 */
#ifndef EOC_POLLER_H
#define EOC_POLLER_H

#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_types.h>
#include <devs/EOC_dev.h>
#include <engine/EOC_scheduler.h>
#include <engine/EOC_handlers.h>
#include <config/EOC_config.h>
#include <db/EOC_db.h>

#define TICKS_BETW_REQ 2

class EOC_poller{
private:
    EOC_db *db;
    EOC_scheduler *sch;
    EOC_config *cfg;
    request_handler_t req_hndl[REQUEST_QUAN];
    response_handler_t resp_hndl[RESPONSE_QUAN];
public:
    EOC_poller(EOC_config *c){
	int i;
	db = new EOC_db;
	sch = new EOC_scheduler(TICKS_BETW_REQ);
	cfg = c;
	for(i=0;i<REQUEST_QUAN;i++)
	    req_hndl[i] = NULL;
	for(i=0;i<RESPONSE_QUAN;i++)
	    resp_hndl[i] = NULL;
    }
    ~EOC_poller(){
	delete db;
	delete sch;
    }
    inline void link_state(EOC_dev::Linkstate link){ sch->link_state(link); }
    inline EOC_dev::Linkstate link_state(){ 
	return ((sch->state()==EOC_scheduler::Offline) ? EOC_dev::OFFLINE : EOC_dev::ONLINE);
    }

    int register_request(u8 type,request_handler_t h);
    int unregister_request(u8 type);
    int register_response(u8 type,response_handler_t h);
    int unregister_response(u8 type);
    EOC_msg *gen_request();
    int process_msg(EOC_msg *m);
};

#endif
