#ifndef EOC_ENGINE_H
#define EOC_ENGINE_H

#include <devs/EOC_dev.h>
#include <devs/EOC_dev_master.h>

#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>

#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>
#include <config/EOC_config.h>

class EOC_engine{
protected:
    dev_type type;
    EOC_config *cfg;
    EOC_router *rtr;
    EOC_responder *resp;
    EOC_poller *poll;
    u16 recv_max;
public:
    EOC_engine(EOC_dev_master *d1,int ticks_p_min=0,u16 rmax = 10);
    EOC_engine(EOC_dev *d1,int ticks_p_min=0,u16 rmax = 10);
    EOC_engine(EOC_dev *d1,EOC_dev *d2, u16 rmax = 10);
    int register_handlers();
    inline ~EOC_engine(){
	if( rtr ) delete rtr;
	if( resp ) delete resp;
	if( poll ) delete poll;
	if( cfg ) delete cfg;
    }
    int setup_state();
    
    int schedule();
};

#endif
