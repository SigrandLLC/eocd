#ifndef EOC_LOOP_H
#define EOC_LOOP_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <generic/EOC_types.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <utils/EOC_ring_container.h>
#include <eoc_debug.h>

class counters_elem{
 public:
    time_t tstamp;
    counters_t cntrs;

    counters_elem(){
		memset(&cntrs,0,sizeof(cntrs));
		tstamp = time(NULL);
    }
    int addit(counters_t cnt){
		//	printf("addit: %d to es\n",cnt.es);
		cntrs.es += cnt.es;
		cntrs.ses += cnt.ses;
		cntrs.crc += cnt.crc;
		cntrs.losws += cnt.losws;
		cntrs.uas += cnt.uas;
    }
	void reset(){
		PDEBUG(DERR,"reset");
		memset(&cntrs,0,sizeof(cntrs));
		tstamp = time(NULL);
	}
};

typedef struct {
    time_t tstamp;
    s32 loop_attn;
    s32 snr_marg;
    shdsl_status_t stat;
    counters_elem elem;
} shdsl_current;


#define EOC_15MIN_SECS 15*60
#define EOC_1DAY_SECS 24*60*60
#define EOC_15MIN_INT_LEN 15
#define EOC_15MIN_INTS 96
#define EOC_1DAY_INTS 30

class EOC_loop{
 public:
 protected:
    // current situation
    shdsl_current state;
	counters_elem tstate;
    EOC_ring_container<counters_elem> _15min_ints;
    EOC_ring_container<counters_elem> _1day_ints;
	u8 is_first_msg;
    side_perf last_msg;
	char lstate; // link state
	time_t moni_ts; // last moniSecs refresh
    
    // TODO: check how hardware maintain moulo counters: i.e. if counter overflow - would it count zero as addit count?
    inline u8 modulo_diff(u8 val,u8 nval,u8 modulo,char *type){
		u32 ret = ((nval-val>=0) ? nval-val : nval + (modulo-val)+1);
		//	printf("modulo_diff8(%s): val(%u) nval(%u) modulo(%u) ret(%u)\n",type,val,nval,modulo,ret);
		return ret;	
    }

    inline u16 modulo_diff(u16 val,u16 nval,u16 modulo,char *type){
		u32 ret = ((nval-val>=0) ? nval-val : nval + (modulo-val)+1);
		//	printf("modulo_diff16(%s): val(%u) nval(%u) modulo(%u) ret(%u)\n",type,val,nval,modulo,ret);
		return ret;	
    }

    inline int int_diff(u32 val,u32 nval,u32 modulo,char *type){
		if( val > nval )
			return 0;
		if( (nval-val)/modulo )
			return modulo;
		return (nval-val)%modulo;
    }

    void status_diff(side_perf *info,counters_t &cntrs);
    int setup_cur_status(side_perf *info);
    int get_localtime(time_t *t,struct tm &ret);
    void shift_rings();
 public:
    EOC_loop();
	// TODO: write    ~EOC_loop();
    int short_status(s8 snr_margin);
    int full_status(side_perf *info,int rel = 0);

    // output interface
    s32 cur_snr(){ return state.snr_marg; }
    s32 cur_attn(){ return state.loop_attn; }
    shdsl_status_t cur_status(){ return state.stat; }
    counters_t cur_counters(){ return state.elem.cntrs; }
    counters_t cur_tcounters(){ return tstate.cntrs; }
    time_t cur_ttstamp(){ return tstate.tstamp; }
    int m15_counters(int index,counters_elem &cntrs){
		if( !(index < EOC_15MIN_INTS && index>=0) )
			return -1;
		if( !_15min_ints[index] )
			return -1;
		cntrs = *_15min_ints[index];
		return 0;
    }

    int m15_nx_counters(int &index,counters_elem &cntrs){
		if( !(index < EOC_15MIN_INTS && index>=0) )
			return -1;
		int i = index;
		while( !_15min_ints[i] && i<EOC_15MIN_INTS )
			i++;
		if( !_15min_ints[i] )
			return -1;
		cntrs = *_15min_ints[i];
		index = i;
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

    int d1_nx_counters(int &index,counters_elem &cntrs){
		if( !(index < EOC_1DAY_INTS && index>=0) )
			return -1;
		int i = index;
		while( !_1day_ints[i] && i<EOC_1DAY_INTS )
			i++;
		if( !_1day_ints[i] )
			return -1;
		cntrs = *_1day_ints[i];
		index = i;
		return 0;
    }

	// Link status change handling
	inline void link_up(){
		PDEBUG(DFULL,"LINK IS UP!!!!!!!!!!!!!!!!");
		if( lstate )
			return;
		lstate = 1;
		is_first_msg = 1;
		moni_ts = time(NULL);
	}
	inline void link_down(){
		PDEBUG(DFULL,"LINK IS DOWN!!!!!!!!!!!!!!!!");
		if( !lstate )
			return;
		lstate  = 0;
	}

	void update_mon_sec()
	{
		time_t cur = time(NULL);
		

		if( !lstate )
			return;
		
		if( (_15min_ints[0]->tstamp + EOC_15MIN_SECS) > moni_ts ){
			if( (_15min_ints[0]->tstamp + EOC_15MIN_SECS) < cur ){ 
				// if 15min interval exceeds. 
				// If also 1day exceeds => 15min too - so dont need to chech 1day int
				int bkp = cur;
				cur = _15min_ints[0]->tstamp + EOC_15MIN_SECS;			
				_15min_ints[0]->cntrs.mon_sec += cur-moni_ts;
				_1day_ints[0]->cntrs.mon_sec += cur-moni_ts;
				moni_ts = cur;
				PDEBUG(DFULL,"Int exceeds: cur=%d,tmp_cur=%d\nmoni_ts=%d",bkp,cur,moni_ts);
			}else{
				PDEBUG(DFULL,"Normal: cur=%d, moni_ts=%d",cur,moni_ts);
				_15min_ints[0]->cntrs.mon_sec += cur-moni_ts;
				_1day_ints[0]->cntrs.mon_sec += cur-moni_ts;
				moni_ts = cur;
			}
			return;
		}

		if( (_1day_ints[0]->tstamp + EOC_1DAY_SECS) > moni_ts ){
			if( (_1day_ints[0]->tstamp + EOC_1DAY_SECS) > cur){
				_1day_ints[0]->cntrs.mon_sec += cur-moni_ts;
				moni_ts = cur;
			}else{
				cur = _1day_ints[0]->tstamp + EOC_1DAY_SECS;
				_1day_ints[0]->cntrs.mon_sec += cur-moni_ts;
				moni_ts = cur;
			}	
		}
	}


	inline void reset_tcounters(){
		PDEBUG(DERR,"Reset Counters");
		tstate.reset();
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
