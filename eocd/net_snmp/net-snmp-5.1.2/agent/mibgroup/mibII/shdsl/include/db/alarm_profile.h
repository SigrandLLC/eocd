#ifndef SIGRAND_ALARM_PROFILE_H
#define SIGRAND_ALARM_PROFILE_H

#include "eoc_primitives.h"
#include

typedef struct {
    char name[ADMIN_STR_LEN];
    char tresh_loop_att;
    char tresh_snr_marg;
    perf_stat_t tresh;
} alarm_prof_t;

#endif