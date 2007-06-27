#ifndef SIGRAND_DB_SPAN_H
#define SIGRAND_DB_SPAN_H
#include "unit.h"
#include "eoc_primitives.h"

typedef struct {
    //---------- Configuration ----------------//
    // Number of repeaters:
    int rnum; 
    // Span Configuration profile
    int conf;
    // Span Alarm profile
    int aconf;
    //---------- Status ----------------//
    // Number of actual repeaters in span
    unsigned int avail_rnum;
    // Maximum attainable rate in span
    unsigned int max_att_lrate;
    // Actual line rate (should equal ifSpeed) in span
    int act_lrate;
    // Transmission mode - Annex
    transm_mode_t tmode;
    
    //---------- Units -----------------//
    unit_t *units[10];
} span_t;


#define span_payload_rate(lrate) ((lrate > OVERHEAD ) ? (lrate-OVERHEAD) : 0 )
#define ind_to_unit(ind) (ind>=0 && ind<=9) ? (ind+1) : -1

// initialisation
span_t *span_alloc();
int span_free(span_t *);
// unit 
int span_add_unit(int inv_num);
unit_t *span_get_unit(int inv_num);

#endif
