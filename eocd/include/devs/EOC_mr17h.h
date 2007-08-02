#ifndef SIGRAND_EOC_SG17_H
#define SIGRAND_EOC_SG17_H

#include <devs/EOC_dev_terminal.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_responses.h>

inline int
mr17h_conf_dir(char *name,char *buf,int max_size){
    return snprintf(buf,max_size,"/sys/class/net/%s/sg17_private",name);    
}

class EOC_mr17h : public EOC_dev_terminal{
protected:

    //DEBUG
    side_perf perf;
    int perf_changed;
    
public:
    EOC_mr17h(char *_ifname);
    ~EOC_mr17h();
    
    // EOC message send/receive
    int send(EOC_msg *m);
    EOC_msg *recv();

    // Span status
    Linkstate link_state();

    // Device configuration
    int set_config_elem(char *name,char *val);
    span_conf_profile_t *cur_config();
    int configure(span_conf_profile_t &cfg);
    int configure();

    // Statistic info
    // DEBUG
    u8 loops() { return 1;};

    int tresholds(s8 loopattn,s8 snr){
	printf("loop_attn(%d) snr_marg(%d)\n",loopattn,snr);
    }

    int status_collect(){
	    return 0;
    }

    int perf_change(u8 loop){
	if(perf_changed){
	    perf_changed = 0;
	    return 1;
	}
	return 0;
    }
	    
    
    u8 losws_alarm(u8 loop){
	return perf.losws_alarm;
    }
    
    u8 loop_attn_alarm(u8 loop){
	return perf.loop_attn_alarm;
    }
    
    u8 snr_marg_alarm(u8 loop){
	return perf.snr_marg_alarm;
    }
    u8 dc_cont_flt(u8 loop){
	return perf.dc_cont_flt;
    }
    u8 dev_flt(u8 loop){
	return perf.dev_flt;
    }
    u8 pwr_bckoff_st(u8 loop){
	return perf.pwr_bckoff_st;
    }
    
    s8 snr_marg(u8 loop){
	return perf.snr_marg;
    }
    s8 loop_attn(u8 loop){
	return perf.loop_attn;
    }
    
    u8 es(u8 loop){
	return perf.es;
    }
    u8 ses(u8 loop){
	return perf.ses;
    }
    u8 crc(u8 loop){
	return perf.crc;
    }
    u8 losws(u8 loop){
	return perf.losws;
    }
    u8 uas(u8 loop){
	return perf.uas;
    }
    u8 pwr_bckoff_base_val(u8 loop){
	return perf.pwr_bckoff_base_val;
    }
    u8 cntr_rst_scur(u8 loop){
	return perf.cntr_rst_scur;
    }
    u8 cntr_ovfl_stur(u8 loop){
	return perf.cntr_ovfl_stur;
    }
    u8 cntr_rst_scuc(u8 loop){
	return perf.cntr_rst_scuc;
    }
    u8 cntr_ovfl_stuc(u8 loop){
	return perf.cntr_ovfl_stuc;
    }
    u8 pwr_bkf_ext(u8 loop){
	return perf.pwr_bkf_ext;
    }
/*    
    shdsl_config config(){ shdsl_config i; return i; }
    int config(shdsl_config cfg){ 
	printf("SET DEVICE: Rate=%d %s annex%d",cfg.lrate, (cfg.master) ? "master" : "slave",cfg.annex);
	return 0;
    }
*/    
};

#endif
