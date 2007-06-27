#ifndef SIGRAND_EOC_RESPONDER_H
#define SIGRAND_EOC_RESPONDER_H

#include <devs/EOC_dev.h>
#include <engine/EOC_router.h>
#include ????EOC_msg_proc.h"????

/*
    class EOC_responder:
	Defines universal EOC code part which runs on ewery unit in
	SHDSL span.
	It provides message sending/receiving/processing/routing.
*/

class EOC_responder{
    EOC_router *rout;
    EOC_dev *dev;
    EOC_msg_proc *proc;
public:
    EOC_responder(EOC_dev::type t);
    ~EOC_responder();
    int check_channel();
    inline int send_message(eoc_msg *m);
//    inline EOC_proc *get_router(){ return rout; }
};

#endif