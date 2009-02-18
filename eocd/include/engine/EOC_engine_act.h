#ifndef EOC_ENGINE_ACT_H
#define EOC_ENGINE_ACT_H

// TODO: delete - debug
#include <sys/types.h>
#include <dirent.h>
//-----------------------

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
    EOC_engine_act(EOC_dev_terminal *d1,EOC_config *cfg,EOC_dev::dev_del_func df = NULL,u16 ticks_p_min=0,u16 rmax = 10);
    inline ~EOC_engine_act(){
    	PDEBUG(DFULL,"destructor");
		if( poll ){
			delete poll;
			poll = NULL;
		}
    }

    int setup_state_act();
    int schedule(char *ch_name);
    int register_handlers();
    int app_request(app_frame *fr);
	// virtual function of EOC_engine
	int local_configure(int &ch){
		PDEBUG(DERR,"START");
		ch = 0;
	}
};

#endif
