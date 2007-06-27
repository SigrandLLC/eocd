#ifndef SIGRAND_DB_UNIT_H
#define SIGRAND_DB_UNIT_H
#include "endp.h"
#include "eoc_primitives.h"

typedef struct {
//---- G.991.2, Section 9.5.5.7.4, Inventory response (ID 130) ----//
    // Vendor serial number    
    char vend_id[8];
    // Vendor serial number    
    char vend_model[12];
    // Vendor serial number
    char vend_sn[12];
    // Vendor list number
    char vend_list_num[3];
    // Vendor issue number
    char vend_issue_num[2];
    // Vendor software version
    char vend_sftw_ver[6];
    // Equipment Code
    char eq_code[10];
    // Other vendor information
    char vend_other[12];

//---- G.991.2, Section 9.5.5.7.2, Discovery response (ID 129) ----//
    // Vendor EOC software version
    int vend_eoc_ver;    
    // Version of the HDSL2/SHDSL standard implemented
    int standard_ver;

    transm_mode_t transm_cap;

//----- UnitMaintenanceEntry ----------------//
    int loopb_to;
    pwr_src_t psrc;

    endp_t *endps[2];
} unit_t;

#define ind_to_endp(ind) (ind>=0 && ind<=1) ? (ind+1) : -1

unit_t *unit_alloc();
int unit_free(unit_t *);

#endif
