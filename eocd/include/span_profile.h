#ifndef SPAN_PROFILE_H
#define SPAN_PROFILE_H
#include <generic/EOC_types.h>
#include <utils/hash_table.h>

class conf_profile : public hash_elem{
public:
    u16 wires : 3;
    u16 annex : 4;
    u16 power :2;
    u16 ref_clk : 2;
    u16 line_probe :1;
    u32 min_rate;
    u32 max_rate;
};

#endif
