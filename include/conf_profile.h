#ifndef SPAN_PROFILE_H
#define SPAN_PROFILE_H

#include <generic/EOC_types.h>
#include <utils/hash_table.h>
#include <generic/span_conf_type.h>
#include <devs/EOC_dev.h>

#define EOC_DEBUG
#include <generic/EOC_generic.h>

struct span_compat_info {
	tcpam_t tcpam_max;
	int rate_max;
};

static struct span_compat_info comp_levels[] = {
	{	tcpam_max : tcpam32, rate_max : 5696},
	{	tcpam_max : tcpam128, rate_max : 14144},
};
#define COMP_LEV_NUM sizeof(comp_levels)/sizeof(struct span_compat_info)

class conf_profile: public hash_elem {
public:
	span_conf_profile_t conf;
	typedef enum {
		profRW = 0, profRO
	} prof_access_t;
	EOC_dev::compatibility_t comp;
	prof_access_t access;

	conf_profile() :
		hash_elem() {
		is_updated = 0;
		access = profRW;
		memset(&conf, 0, sizeof(conf));
		conf.annex = annex_a;
		conf.wires = twoWire;
		conf.power = noPower;
		conf.psd = symmetric;
		conf.clk = localClk;
		conf.line_probe = disable;
		conf.remote_cfg = disabled;
		conf.tcpam = tcpam32;

		comp = EOC_dev::comp_base;
	}

	~conf_profile() {
		PDEBUG(DFULL,"destructor");
	}

	int check_comp() {
		if (!((unsigned int) comp < COMP_LEV_NUM)) {
			PDEBUG(DERR, "Comp level = %d > %d", (int) comp, COMP_LEV_NUM);
			return -1;
		}
		PDEBUG(DERR, "tcpam=%d, tcpam_max=%d, rate=%d, rate_max=%d",
			conf.tcpam, comp_levels[(int) comp].tcpam_max, conf.rate,
			comp_levels[(int) comp].rate_max);
		return !((conf.tcpam <= comp_levels[(int) comp].tcpam_max) && (conf.rate
			<= comp_levels[(int) comp].rate_max));
	}


};

#endif
