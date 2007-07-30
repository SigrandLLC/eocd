#ifndef EOC_LOOP_H
#define EOC_LOOP_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <utils/EOC_ring_container.h>

class shdsl_counters{
public:
    time_t tstamp;
    u32 es;
    u32 ses;
    u32 crc;
    u32 losws;
    u32 uas;

    shdsl_counters(){
	es = ses = crc = losws = uas = 0;
	tstamp = 0;
    }
    int addit(shdsl_counters cnt){
	es += cnt.es;
	ses += cnt.ses;
	crc += cnt.crc;
	losws += cnt.losws;
	uas += cnt.uas;
    }
    
};


typedef struct{
    u16 noDefect:1;
    u16 powerBackoff:1;
    u16 deviceFault:1;
    u16 dcContFault:1;
    u16 snrMargAlarm:1;
    u16 loopAttnAlarm:1;
    u16 loswFailAlarm:1;
    u16 configInitFailure:1;
    u16 protoInitFailure:1;
    u16 noNeighborPresent:1;
    u16 loopbackActive:1;
} shdsl_cur_status;

typedef struct {
    time_t tstamp;
    s32 loop_attn;
    s32 snr_marg;
    shdsl_cur_status stat;
    shdsl_counters cntrs;
} shdsl_current;


#define EOC_15MIN_INT_LEN 1
#define EOC_15MIN_INTS 96
#define EOC_1DAY_INTS 30

class EOC_loop{
public:
protected:
    // current situation
    shdsl_current state;
    EOC_ring_container<shdsl_counters> _15min_ints;
    EOC_ring_container<shdsl_counters> _1day_ints;
    side_perf last_msg;
    
    inline u8 modulo_diff(u8 val,u8 nval,u8 modulo,char *type){
	u32 ret = ((nval-val>=0) ? nval-val : nval + (modulo-val));
	printf("modulo_diff8(%s): val(%u) nval(%u) modulo(%u) ret(%u)\n",type,val,nval,modulo,ret);
	return ret;	
    }

    inline u16 modulo_diff(u16 val,u16 nval,u16 modulo,char *type){
	u32 ret = ((nval-val>=0) ? nval-val : nval + (modulo-val));
	printf("modulo_diff16(%s): val(%u) nval(%u) modulo(%u) ret(%u)\n",type,val,nval,modulo,ret);
	return ret;	
    }

    inline void status_diff(side_perf *info,shdsl_counters &cntrs);
    int setup_cur_status(side_perf *info);
    inline int get_localtime(time_t *t,struct tm &ret);
    void shift_rings();
            
public:
    EOC_loop();
    int short_status(s8 snr_margin);
    int full_status(side_perf *info);
    // TODO: removethis DEBUG
    void print_15m(){
	printf("Dump 15min stat:\n");
	for(int i=0;i<EOC_15MIN_INTS;i++){
	    if( _15min_ints[i] ){
		char *c = ctime(&_15min_ints[i]->tstamp);
		int len = strlen(c);
		c[len-1] = '\0';
		printf("%s: es(%u) ses(%u) crc(%u) losws(%u) uas(%u)\n",
			c,_15min_ints[i]->es,
			_15min_ints[i]->ses,_15min_ints[i]->crc,_15min_ints[i]->losws,
			_15min_ints[i]->uas);
	    }
	}
    }
};

#endif
