#ifndef EOC_CHANNEL_H
#define EOC_CHANNEL_H

#include <generic/EOC_types.h>

#include <utils/hash_table.h>

#include <snmp/snmp-generic.h>

#include <devs/EOC_dev.h>
#include <devs/EOC_dev_terminal.h>

#include <engine/EOC_engine.h>
#include <engine/EOC_engine_act.h>

#define EOC_DEBUG
#include <generic/EOC_generic.h>

class channel_elem: public hash_elem {
protected:
public:
	EOC_engine *eng;
	channel_elem(EOC_dev_terminal *dev, EOC_config *c,EOC_dev::dev_del_func df) {
		PDEBUG(DFULL,"create EOC_engine");
		eng = new EOC_engine(dev, c, slave,df);
	}
	channel_elem(EOC_dev_terminal *dev, EOC_config *c,EOC_dev::dev_del_func df,u32 tick_per_min) {
		EOC_engine_act *tmp = new EOC_engine_act(dev, c, df, tick_per_min);
		eng = (EOC_engine *)tmp;
		PDEBUG(DFULL,"create EOC_engine_act: %p",eng);
	}
	~channel_elem(){
		PDEBUG(DFULL,"destructor");
		if (eng->get_type() == master) {
			PDEBUG(DFULL,"delete EOC_engine_act: %p",eng);
			delete (EOC_engine_act*) eng;
		} else {
			PDEBUG(DFULL,"delete EOC_engine: %p",eng);
			delete eng;
		}
	}
};

#endif
