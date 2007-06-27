#ifndef SIGRAND_CONF_PROF_H
#define SIGRAND_CONF_PROF_H

#include "eoc_primitives.h"
#include "../utils/EOCHtable.h"

class EOCAlarm_elem: public EOCHash_data{
public:
    char tresh_loop_att;
    char tresh_snr_marg;
    perf_stat_t tresh;
    
    status

    EOCAlarm_elem(){
	tresh_loop_att;
	char tresh_snr_marg;
        perf_stat_t tresh;
};

class EOCAlarm{
private:
    EOCHash_table *table;
public:
    EOCConf();
    ~EOCConf();
    int add_prof(char *name);
    int del_prof(char *name);
};


EOCConf::EOCConf()
{
    table = new EOCHash_table<EOCAlarm_elem>;
}

EOCConf::~EOCConf(){
    delete table;
}

int
EOCConf::add_prof(char name[32])
{
    EOCConf_elem *el = new EOCAlarm_elem;
    return table->add(name,32,el);
}


int
EOCConf::del_prof(char name[32])
{
    return table->del(name);
}

#endif
