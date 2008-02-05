#ifndef EOC_ENGINE_ACT_H
#define EOC_ENGINE_ACT_H

#include <devs/EOC_dev.h>
#include <devs/EOC_dev_terminal.h>

#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>

#include <utils/hash_table.h>

#include <config/EOC_config.h>

#include <engine/EOC_poller.h>
#include <engine/EOC_engine.h>



class EOC_engine_act : public EOC_engine {
 protected:
    EOC_poller *poll;
 public:
    EOC_engine_act(EOC_dev_terminal *d1,EOC_config *cfg,u16 ticks_p_min=0,u16 rmax = 10);
    inline ~EOC_engine_act(){
		if( rtr ) delete rtr;
		if( resp ) delete resp;
		if( poll ) delete poll;
    }

    int setup_state_act();
    int schedule();
    int register_handlers();
    int app_request(app_frame *fr);
};

#endif
