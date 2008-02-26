#ifndef SPAN_PROFILE_H
#define SPAN_PROFILE_H

#include <generic/EOC_types.h>
#include <utils/hash_table.h>
#include <generic/span_conf_type.h>


class conf_profile : public hash_elem{
 public:
    span_conf_profile_t conf;
	typedef enum { profRW=0, profRO } prof_access_t;
	prof_access_t access;

 conf_profile() : hash_elem(){
		is_updated = 0;
		access = profRW;
		memset(&conf,0,sizeof(conf));
		conf.annex = annex_a;
		conf.wires = twoWire;
		conf.power = noPower;
		conf.psd = symmetric;
		conf.clk = localClk;
		conf.line_probe = disable;
		conf.remote_cfg = disabled;
		conf.tcpam = tcpam32;
	}
};

#endif
