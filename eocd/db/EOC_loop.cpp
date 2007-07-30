#include <db/EOC_loop.h>

void EOC_loop::
status_diff(side_perf *info,shdsl_counters &cntrs)
{
    if( !(memcmp(info,&last_msg,sizeof(last_msg))) )
        return;
    cntrs.es = modulo_diff(last_msg.es,info->es,255,"es");
    cntrs.ses = modulo_diff(last_msg.ses,info->ses,255,"ses");
    cntrs.crc = modulo_diff(last_msg.crc,info->crc,65535,"crc");
    cntrs.losws = modulo_diff(last_msg.losws,info->losws,255,"losws"); 
    cntrs.uas = modulo_diff(last_msg.uas,info->uas,255,"uas");
    last_msg = *info;
}

int EOC_loop::
setup_cur_status(side_perf *info)
{
    state.stat.loswFailAlarm = info->losws_alarm;
    state.stat.loopAttnAlarm = info->loop_attn_alarm;
    state.stat.snrMargAlarm = info->snr_marg_alarm;
    state.stat.dcContFault = info->dc_cont_flt;
    state.stat.powerBackoff = info->pwr_bckoff_st;
}
    
int EOC_loop::
get_localtime(time_t *t,struct tm &ret){
    struct tm *ptr = localtime(t);
    if( !ptr )
        return -1;
    ret = *ptr;
    return 0;
}

void EOC_loop::
shift_rings(){
    time_t cur,_15m,_1d,t;
    struct tm cur_tm,cur_int_tm,_15m_tm,_1d_tm;
    // get current time stamp
    time(&cur);
    _15m = _15min_ints[0]->tstamp;
    _1d = _1day_ints[0]->tstamp;

    // get tm structures
    if( get_localtime(&cur,cur_tm) || get_localtime(&_15m,_15m_tm) || get_localtime(&_1d,_1d_tm)){
        //TODO eoc_log("Cannot convert time info for 15 min timestamp"); 
        return;
    }    
    cur_int_tm = cur_tm;
    cur_int_tm.tm_min = (((int)cur_int_tm.tm_min)/EOC_15MIN_INT_LEN)*EOC_15MIN_INT_LEN;

    u8 _15m_int = (_15m_tm.tm_min + _15m_tm.tm_hour*60)/EOC_15MIN_INT_LEN;
    u8 _15m_int_cur = (cur_tm.tm_min + cur_tm.tm_hour*60)/EOC_15MIN_INT_LEN;
    int shift_num = modulo_diff(_15m_int,_15m_int_cur,EOC_15MIN_INTS,"15m");
    _15min_ints.shift(shift_num);
    if( shift_num ){
        _15min_ints[0]->tstamp = mktime(&cur_int_tm);
    }
	
    if( _1d_tm.tm_year == cur_tm.tm_year ){
        shift_num = _1d_tm.tm_yday - cur_tm.tm_yday;
        _1day_ints.shift(shift_num);
        if( shift_num ){
	    _1day_ints[0]->tstamp = mktime(&cur_int_tm);
	}		
    }else{
        // TODO : count difference to different years 
    }
}

            
EOC_loop::
EOC_loop() : _15min_ints(EOC_15MIN_INTS) , _1day_ints(EOC_1DAY_INTS) {
    struct tm ctm;
    time_t t;
    memset(&state,0,sizeof(state));
    memset(&last_msg,0,sizeof(last_msg));	
    time(&t);
    if( get_localtime(&t,ctm)){
        // TODO: eoc_log("Error getting time"); 
        return ;
    }
    ctm.tm_min = (((int)ctm.tm_min)/EOC_15MIN_INT_LEN)*EOC_15MIN_INT_LEN;
    _15min_ints[0]->tstamp = mktime(&ctm);
    _1day_ints[0]->tstamp = mktime(&ctm);
}


int EOC_loop::
short_status(s8 snr_margin)
{
    shift_rings();	
    // change online data
    state.snr_marg = snr_margin;
}

int EOC_loop::
full_status(side_perf *info)
{
    shdsl_counters cntrs;
    status_diff(info,cntrs);
    shift_rings();	
    // change online data
    state.loop_attn = info->loop_attn;
    state.snr_marg = info->snr_marg;
    setup_cur_status(info);	
    // Change counters
    state.cntrs.addit(cntrs);
    _15min_ints[0]->addit(cntrs);
    _1day_ints[0]->addit(cntrs);
}
