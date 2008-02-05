#ifndef EOC_CHANNEL_H
#define EOC_CHANNEL_H

#include <generic/EOC_types.h>

#include <utils/hash_table.h>

#include <snmp/snmp-generic.h>

#include <devs/EOC_dev.h>
#include <devs/EOC_dev_terminal.h>

#include <engine/EOC_engine.h>
#include <engine/EOC_engine_act.h>

class channel_elem : public hash_elem{
 protected:
 public:
    EOC_engine *eng;
    channel_elem(EOC_dev_terminal *dev,EOC_config *c){
		eng = new EOC_engine(dev,c);
    }
    channel_elem(EOC_dev_terminal *dev,EOC_config *c,u32 tick_per_min){
		eng = new EOC_engine_act(dev,c,tick_per_min);
    }
    ~channel_elem(){
		if( eng->get_type() == master ){
			delete (EOC_engine_act*)eng;
		}else{
			delete eng;
		}
    }
};

#endif
