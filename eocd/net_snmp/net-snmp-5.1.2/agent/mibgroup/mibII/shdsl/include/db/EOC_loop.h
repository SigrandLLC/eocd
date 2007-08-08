#ifndef EOC_LOOP_H
#define EOC_LOOP_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <generic/EOC_types.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <utils/EOC_ring_container.h>

class counters_elem{
public:
    time_t tstamp;
    counters_t cntrs;

    counters_elem(){
	memset(&cntrs,0,sizeof(cntrs));
	tstamp = 0;
    }
    int addit(counters_t cnt){
	cntrs.es += cnt.es;
	cntrs.ses += cnt.ses;
	cntrs.crc += cnt.crc;
	cntrs.losws += cnt.losws;
	cntrs.uas += cnt.uas;
    }
    
};

typedef struct {
    time_t tstamp;
    s32 loop_attn;
    s32 snr_marg;
    shdsl_status_t stat;
    counters_elem elem;
} shdsl_current;


#define EOC_15MIN_INT_LEN 1
#define EOC_15MIN_INTS 96
#define EOC_1DAY_INTS 30

class EOC_loop{
public:
protected:
    // current situation
    shdsl_current state;
    EOC_ring_container<counters_elem> _15min_ints;
    EOC_ring_container<counters_elem> _1day_ints;
    side_perf last_msg;
    
    // TODO: check how hardware maintain moulo counters: i.e. if counter overflow - would it count zero as addit count?
    inline u8 modulo_diff(u8 val,u8 nval,u8 modulo,char *type){
	u32 ret = ((nval-val>=0) ? nval-val : nval + (modulo-val)+1);
	printf("modulo_diff8(%s): val(%u) nval(%u) modulo(%u) ret(%u)\n",type,val,nval,modulo,ret);
	return ret;	
    }

    inline u16 modulo_diff(u16 val,u16 nval,u16 modulo,char *type){
	u32 ret = ((nval-val>=0) ? nval-val : nval + (modulo-val)+1);
	printf("modulo_diff16(%s): val(%u) nval(%u) modulo(%u) ret(%u)\n",type,val,nval,modulo,ret);
	return ret;	
    }

    void status_diff(side_perf *info,counters_t &cntrs);
    int setup_cur_status(side_perf *info);
    inline int get_localtime(time_t *t,struct tm &ret);
    void shift_rings();
            
public:
    EOC_loop();
// TODO: write    ~EOC_loop();
    int short_status(s8 snr_margin);
    int full_status(side_perf *info);

    // output interface
    s32 cur_snr(){ return state.snr_marg; }
    s32 cur_attn(){ return state.loop_attn; }
    counters_t cur_counters(){ return state.elem.cntrs; }
    shdsl_status_t cur_status(){ return state.stat; }
    int m15_counters(int index,counters_elem &cntrs){
	if( !(index < EOC_15MIN_INTS && index>=0) )
	    return -1;
	if( !_15min_ints[index] )
	    return -1;
	cntrs = *_15min_ints[index];
	return 0;
    }
    
    int d1_counters(int index,counters_elem &cntrs){
	if( !(index < EOC_1DAY_INTS && index>=0) )
	    return -1;
	if( !_1day_ints[index] )
	    return -1;
	cntrs = *_1day_ints[index];
	return 0;
    }


    // TODO: removethis DEBUG
    void print_15m(){
	printf("Dump 15min stat:\n");
	for(int i=0;i<EOC_15MIN_INTS;i++){
	    if( _15min_ints[i] ){
		char *c = ctime(&_15min_ints[i]->tstamp);
		int len = strlen(c);
		c[len-1] = '\0';
		printf("%s: es(%u) ses(%u) crc(%u) losws(%u) uas(%u)\n",
			c,_15min_ints[i]->cntrs.es,
			_15min_ints[i]->cntrs.ses,_15min_ints[i]->cntrs.crc,
			_15min_ints[i]->cntrs.losws,_15min_ints[i]->cntrs.uas);
	    }
	}
    }
};

#endif
