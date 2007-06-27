#ifndef SIGRAND_EOC_ENGINE_H
#define SIGRAND_EOC_ENGINE_H

class EOC_engine{
protected:
    EOC_role type;
    EOC_dev_cfg *cfg;
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

    int schedule();
};

#endif