#ifndef EOC_ENGINE_H
#define EOC_ENGINE_H

#include <devs/EOC_dev.h>
#include <devs/EOC_dev_terminal.h>
#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <config/EOC_config.h>

#define RECV_ONCE 10

class EOC_engine{
 protected:
    dev_type type;
    EOC_router *rtr;
    EOC_responder *resp;
    EOC_config *cfg;
    u16 recv_max;
 public:
    EOC_engine(EOC_dev_terminal *d1,EOC_config *c,dev_type t = slave,u16 rmax = RECV_ONCE);
    EOC_engine(EOC_dev *d1,EOC_dev *d2, u16 rmax = RECV_ONCE);
    inline ~EOC_engine(){
		if( rtr ) delete rtr;
		if( resp ) delete resp;
		if( cfg ) delete cfg;
    }
	EOC_config *config(){ return cfg; }
    int setup_state();
    virtual int schedule();
    dev_type get_type(){ return type; }
    int configure(char *ch_name); // Slave configuration, Repeater need no configuration
};

#endif
