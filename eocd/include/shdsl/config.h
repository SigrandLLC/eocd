#ifndef EOC_SHDSL_CONFIG_H
#define EOC_SHDSL_CONFIG_H

#include <generic/EOC_types.h>

class shdsl_config {
public:
    u32 max_rate : 20;
    u32 min_rate : 20;
    u32 master : 1;
    u32 annex  :4;
};

#endif
