#ifndef SIGRAND_EOC_ENGINE_H
#define SIGRAND_EOC_ENGINE_H

#include <generic/EOC_generig.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>

class EOC_engine{
protected:
    dev_type type;
//    EOC_dev_cfg *cfg;
    EOC_router *rtr;
    EOC_responder *resp;
    EOC_poller *poll;
public:
    EOC_engine(EOC_role r,EOC_dev *d1,EOC_dev *d2);
    inline ~EOC_engine(){
	delete rtr;
	delete resp;
	delete p;
    }
    void setup_state();
};

#endif