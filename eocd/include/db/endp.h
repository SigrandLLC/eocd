#ifndef SIGRAND_DB_ENDP_H
#define SIGRAND_DB_ENDP_H

#include "pair.h"
#include "eoc_primitives.h"


typedef struct {
//----- Configuration ---------------//
    // The side of the unit associated with this segment endpoint
    side_t side;

//----- EndpointMaintEntry ----------------//
    loopback_cfg_t loopb_cfg;
    tip_ring_rev_t tip_ring;
    power_backoff_t pwr;
    soft_restart_t rst;
    
    pair_t *pairs[4];
} endp_t;

#define ind_to_pair(ind) (ind>=0 && ind<=3) ? (ind+1) : -1


#endif
