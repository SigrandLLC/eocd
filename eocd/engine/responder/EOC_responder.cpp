#include "EOC_responder.h"

EOC_responder::EOC_responder(EOC_dev::type t)
{
    dev = new EOC_dev(t);
    rout = new EOC_router(dev);
    proc = new EOC_msg_proc(dev);
}

EOC_responder::~EOC_responder()
{
    delete proc;
    delete rout;
    delete dev;
}

int 
EOC_responder::check_channel()
{
    eoc_msg *m;
    int error;

    if( !(m = rout->receive(&error)) )
	return error;
    if( !(m = proc->process(m,&error)) )
	return error;
    return rout->send(m);
}

inline int
EOC_responder::send_message(eoc_msg *m){
    return rout->send(m);
}