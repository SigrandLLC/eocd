#ifndef EOC_DUMMY_1_H
#define EOC_DUMMY_1_H
#include<string.h>
#include<stdio.h>

#include <devs/EOC_dev_master.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_responses.h>

class dummy_channel{
protected:
    struct{
	char buf[256];
	int len;
    } chan[256];
    int head,tail;
    inline int inc(int ind){
	if( ind+1 < 256 )
	    return ind+1;
	return 0;
    }
public:
    dummy_channel(){
	head = 0;
	tail = 0;
    }
    inline int enqueue(char *m,int col){
	if( inc(tail) == head )
	    return -1;
	memcpy(chan[tail].buf,m,col);
	chan[tail].len = col;
	tail = inc(tail);
	return 0;
    }

    inline int dequeue(char *m,int *col){
	if( head == tail )
	    return -1;
	memcpy(m,chan[head].buf,chan[head].len);
	*col = chan[head].len;
	head = inc(head);
	return 0;
    }
    
};



class EOC_dummy1 : public EOC_dev_master{
protected:
    char name[256];
    dummy_channel *snd,*rcv;
    int valid;
    side_perf perf;
    int perf_changed;
public:
    EOC_dummy1(char *name,dummy_channel *snd,dummy_channel *rcv);
    ~EOC_dummy1();    

    int send(EOC_msg *m);
    EOC_msg *recv();

    Linkstate link_state();

    int setup_current_stat(side_perf p){
	perf = p;
	perf_changed = 1;
    }

    side_perf get_current_stat(){
	return perf;
    }
    
    u8 loops() { return 1;};

    int tresholds(s8 loopattn,s8 snr){
	printf("%s: loop_attn(%d) snr_marg(%d)\n",name,loopattn,snr);
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
    
    shdsl_config config(){ shdsl_config i; return i; }
    int config(shdsl_config cfg){ 
	printf("SET DEVICE: Rate=%d %s annex%d",cfg.lrate, (cfg.master) ? "master" : "slave",cfg.annex);
	return 0;
    }
};

#endif



/*
int main()
{
    char a[256];
    int b;
    dummy_channel d;
    d.enqueue("aaaaaaaaaa",10);
    d.enqueue("aaaaaaaaaaa",11);
    d.enqueue("aaaaaaaaaaaa",12);
    d.enqueue("aaaaaaaaaaaaaaa",15);
    d.enqueue("aaaaa",5);
    d.enqueue("aaaaaa",6);    
    d.enqueue("aaaaaaaaaa",10);    
    d.enqueue("aaaaaaaaaaa",11);
    
    while( !d.dequeue(a,&b) ){
	a[b] = 0;
	printf("(%d): %s\n",b,a);
    }
}
*/
