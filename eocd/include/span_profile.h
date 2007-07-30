#ifndef SPAN_PROFILE_H
#define SPAN_PROFILE_H
#include <generic/EOC_types.h>
#include <utils/hash_table.h>

typedef enum { annex_a=1,annex_b } annex_t;
typedef enum { twoWire = 1,fourWire,sixWire,eightWire } wires_t;      
typedef enum { noPower = 1, powerFeed, wettingCurrent } power_t;
typedef enum { symmetric = 1, asymmetric } psd_t;
typedef enum { localClk = 1, networkClk, dataOrNetworkClk, dataClk } clk_t;
typedef enum { disable = 1, enable } line_probe_t;
typedef enum { disabled = 1, enabled } remote_cfg_t; 


typedef struct{
    annex_t annex;
    wires_t wires;      
    power_t power;
    psd_t psd;    
    clk_t clk;    
    line_probe_t line_probe;
    remote_cfg_t remote_cfg; 
    
    u8 use_cur_down :1;                                                   
    u8 use_worst_down :1;                                                  
    u8 use_cur_up :1;                                                     
    u8 use_worst_up :1;

    u32 min_rate;
    u32 max_rate;
    s32 cur_marg_down;
    s32 worst_marg_down;
    s32 cur_marg_up;
    s32 worst_marg_up;
} span_conf_profile_t;

class conf_profile : public hash_elem{
public:
    span_conf_profile_t conf;

    conf_profile() : hash_elem()
    {
	memset(&conf,0,sizeof(conf));
	conf.annex = annex_a;
	conf.wires = twoWire;
	conf.power = noPower;
	conf.psd = symmetric;
	conf.clk = localClk;
	conf.line_probe = disable;
	conf.remote_cfg = disabled;
    }
};

#endif
