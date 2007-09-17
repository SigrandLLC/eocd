#ifndef EOC_CONFIG_H
#define EOC_CONFIG_H

#include <generic/EOC_types.h>
#include <utils/hash_table.h>

#include <conf_profile.h>

class EOC_config{
    hash_table *conf_prof;
    hash_table *alarm_prof;
    char *cprof_name;
    char *aprof_name;
    u16 rep_num;
public:
    EOC_config(hash_table *c,hash_table *a,char *cn,char *an,u16 rep)
    {
	conf_prof = c;
	alarm_prof = a;
	cprof_name = cn;
	aprof_name = an;
	rep_num = rep;
    }
    const char *conf_prof_name(){return cprof_name;}
    const char *alarm_prof_name(){return cprof_name;}
    u16 repeaters(){ return rep_num; }
    hash_table *conf_tbl(){ return conf_prof; }
    hash_table *alarm_tbl(){ return alarm_prof; }
    s8 snr_tresh(){ return 0;}
    s8 loop_tresh(){ return 0; }
};

#endif
