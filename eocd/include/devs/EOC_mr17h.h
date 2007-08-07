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
    int set_dev_option(char *name,char *val);
    int get_dev_option(char *name,char *&buf);

    span_conf_profile_t *cur_config();
    int configure(span_conf_profile_t &cfg);
    int configure();

    // Statistic info
    // DEBUG
    int loops() { return 1;};

    int tresholds(s8 loopattn,s8 snr){
	printf("loop_attn(%d) snr_marg(%d)\n",loopattn,snr);
    }
    
    int statistics(int loop,side_perf &stat);

};

#endif
