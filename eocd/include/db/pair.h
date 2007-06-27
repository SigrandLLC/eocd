#ifndef SIGRAND_DB_PIRE_H
#define SIGRAND_DB_PIRE_H

#include "pair.h"
#include "eoc_primitives.h"
#include "../utils/stat_ring.h"

typedef struct{
    unsigned int moni_sec;
    perf_stat_t perf_stat;
} day_int_t;
					


typedef struct {
//----- EndpointConfEntry ---------------//
    // The wire pair of the modem associated with this segment endpoint
    pair_num_t pair;
    // Alarm configuration profile
    int aprof;
//----- EndpointCurrEntry ---------------//
    // Current line attenuation (-127 .. 128)
    char cur_attn;
    // Current SNR margin (-127 .. 128)
    char cur_snr;
    // Contains the current state of the endpoint
    status_bits_t bits;
    // Perfomance & statistic counters since last restart
    perf_stat_t stat;
    // Current 15 minutes perfomance & statistic counters
    unsigned short c15m_elaps;
    perf_stat_t *c15m_stat;
    // Current 1 day perfomance & statistic counters
    unsigned short c1d_elaps;
    perf_stat_t *c1d_stat;
    //------------------------
    tip_ring_rev_t tip_ring;
    act_state_t act_st;

//---- 15MinIntervalEntry ---------//
    Eocd_ring *i15min;

//---- 1DayIntervalEntry ---------//
    Eocd_ring *i1day;
    
} pair_t;


#endif
