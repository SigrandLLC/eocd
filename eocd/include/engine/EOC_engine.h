#ifndef EOC_ENGINE_H
#define EOC_ENGINE_H

// TODO: delete - debug
#include <sys/types.h>
#include <dirent.h>
//-----------------------

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
	EOC_dev *dev1,*dev2;
    EOC_router *rtr;
    EOC_responder *resp;
    EOC_config *cfg;
    u16 recv_max;
 public:
    EOC_engine(EOC_dev_terminal *d1,EOC_config *c,dev_type t = slave,u16 rmax = RECV_ONCE);
    EOC_engine(EOC_dev *d1,EOC_dev *d2, u16 rmax = RECV_ONCE);
    inline ~EOC_engine(){
    	PDEBUG(DFULL,"delete responder");
		if( resp ){
			delete resp;
			resp = NULL;
		}
    	PDEBUG(DFULL,"delete config");
    	if( cfg ){
    		delete cfg;
    		cfg = NULL;
    	}
    	PDEBUG(DFULL,"delete router");
		if( rtr ){
			delete rtr;
			rtr = NULL;
		}
    	PDEBUG(DFULL,"end");
    }
	EOC_config *config(){ return cfg; }
    int setup_state();
    virtual int schedule(char *chname);
    dev_type get_type(){ return type; }
	virtual int local_configure(int &ch){ch=0; return 0;}
    int configure(char *ch_name); // Slave configuration, Repeater need no configuration
    EOC_dev::compatibility_t get_compat(){
    	if( type == repeater )
    		return EOC_dev::comp_base;
    	return dev1->comp();
    }
    int check_compat(){
    	// Slave & repeaters are always compatible with profile.
    	// They have no much to do with profiles
    	if( type != master ){
    		return 0;
    	}
    	// device compat info should be equal or grater than profiles compat info
    	conf_profile *prof = (conf_profile*) cfg->conf();
    	if (dev1->comp() < prof->comp) {
    		return -1;
    	}
    	return 0;
    }
    const char *dev1_name(){
    	static char buf[32];
    	if( type==repeater)
    		return NULL;
    	EOC_dev_terminal *d = (EOC_dev_terminal*)dev1;
    	strncpy(buf,d->if_name(),32);
    	return buf;
    }
};

#endif
