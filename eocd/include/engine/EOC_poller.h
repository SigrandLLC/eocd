/*
 * EOC_poller.h:
 * 	master EOC polling class
 */
#ifndef EOC_POLLER_H
#define EOC_POLLER_H

#include <generic/EOC_msg.h>
#include <engine/EOC_scheduler.h>
#include <engine/EOC_db.h>

#define REQUEST_QUAN 128
typedef EOC_msg *(*request_handler_t)(EOC_db *db,unit src,unit dst,unsigned char type);
#define RESPONSE_QUAN 128
typedef int (*response_handler_t)(EOC_db *d,EOC_msg *m);


class EOC_poller{
private:
    EOC_db *db;
    EOC_scheduler *sch;
    request_handler_t req_hndl[REQUEST_QUAN];
    response_handler_t resp_hndl[RESPONSE_QUAN];
public:
    EOC_poller(){
	db = new EOC_db;
	sch = new EOC_scheduler;
	for(int i=0;i<HANDLERS_QUAN;i++)
	    handlers[i] = NULL;
    }
    ~EOC_poller(){
	delete db;
	delete sch;
    }
    int register_request(int type,request_handler_t h);
    int unregister_request(int type);
    int register_response(int type,response_handler_t h);
    int unregister_response(int type);
    EOC_msg *gen_request();
    int process_msg(EOC_msg *m);
};

#endif
