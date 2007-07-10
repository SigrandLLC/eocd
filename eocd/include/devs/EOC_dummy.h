#ifndef SIGRAND_EOC_SG17_H
#define SIGRAND_EOC_SG17_H

#include <devs/EOC_dev.h>
#include <generic/EOC_msg.h>

#define HDLC_BUFF_SZ 112

class EOC_dummy : public EOC_dev{
protected:
    char *f1,*f2;
    int valid;
    u8 loop_attn_atr,snr_marg_atr;
public:
    EOC_dummy(char *file1,char *file2);
    ~EOC_dummy();    
    int send(EOC_msg *m);
    EOC_msg *recv();
    Linkstate link_state();
    int tresholds(u8 lattn,u8 snr){
	loop_attn_atr = lattn;
	snr_marg_atr = snr;
    }
    u8 loop_attn(){ return loop_attn_atr; }
    u8 snr_marg(){return snr_marg_atr; } 
};

#endif
