#ifndef SIGRAND_EOC_DEV_MASTER_H
#define SIGRAND_EOC_DEV_MASTER_H

#include <generic/SHDSL_config.h>
#include <devs/EOC_dev.h>

class EOC_dev_master: public EOC_dev{
protected:
public:
    virtual shdsl_config config() = 0;                                                                                         
    virtual int config(shdsl_config cfg) = 0; 
};


#endif
