#ifndef EOC_CHANNEL_H
#define EOC_CHANNEL_H
#include <generic/EOC_types.h>
#include <utils/hash_table.h>
#include <snmp/snmp-generic.h>
#include <devs/EOC_dev.h>
#include <devs/EOC_dev_master.h>

#include <engine/EOC_engine.h>
#include <engine/EOC_engine_act.h>

class channel_elem : public hash_elem{
public:
    EOC_engine *eng;
    channel_elem(EOC_dev *dev){
	eng = new EOC_engine(dev);
    }
    channel_elem(EOC_dev_master *dev,EOC_config *cfg,u32 tick_per_min){
	eng = new EOC_engine_act(dev,cfg,tick_per_min);
    }
};

#endif