#ifndef SIGRAND_CONF_PROF_H
#define SIGRAND_CONF_PROF_H

#include "eoc_primitives.h"
#include "../utils/EOCHtable.h"

class EOCConf_elem : public EOCHash_data{
public:
    char name[ADMIN_STR_LEN];
    int hash;
    wire_if_t wire_if;
    unsigned int min_lrate;
    unsigned int max_lrate;
    PSD_t psd;
    transm_mode_t transm_mode;
    remote_conf_t rconf;
    pwr_feeding_t pwd_feed;
    char cur_marg_down;
    char worst_marg_down;
    char cur_marg_up;
    char worst_marg_up;
    targ_marg_t tmarg;
    clock_ref_t clk_ref;
    line_probe_t lprobe;
};

class EOCConf{
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
    table = new EOCHash_table;
}

EOCConf::~EOCConf(){
    delete table;
}

int
EOCConf::add_prof(char name[32])
{
    EOCConf_elem *el = new EOCConf_elem;
    return table->add(name,32,el);
}


int
EOCConf::del_prof(char name[32])
{
    return table->del(name);
}

#endif