#ifndef EOC_ENGINE_H
#define EOC_ENGINE_H

#include <devs/EOC_dev.h>
#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>

class EOC_engine{
protected:
    dev_type type;
    EOC_router *rtr;
    EOC_responder *resp;
    u16 recv_max;
public:
    EOC_engine(EOC_dev *d1,u16 rmax = 10);
    EOC_engine(dev_type t,EOC_dev *d1,u16 rmax = 10);
    EOC_engine(EOC_dev *d1,EOC_dev *d2, u16 rmax = 10);
    inline ~EOC_engine(){
	if( rtr ) delete rtr;
	if( resp ) delete resp;
    }
    int setup_state();
    virtual int schedule();
    dev_type get_type(){ return type; }
};

#endif
