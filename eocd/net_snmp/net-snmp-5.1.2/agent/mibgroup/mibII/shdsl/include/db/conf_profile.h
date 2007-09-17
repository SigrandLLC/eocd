#ifndef SIGRAND_CONF_PROFILE_H
#define SIGRAND_CONF_PROFILE_H

#include "eoc_primitives.h"

typedef struct {
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
} conf_prof_t;


#endif