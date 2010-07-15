#define EOC_DEBUG
#include <eoc_debug.h>


#include <db/EOC_loop.h>


void EOC_loop::
status_diff(side_perf *info,counters_t &cntrs)
{
    memset(&cntrs,0,sizeof(cntrs));
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
    int _15m_int = (_15m_tm.tm_min + _15m_tm.tm_hour*60 + _15m_tm.tm_yday*60*24 )/EOC_15MIN_INT_LEN;
    int _15m_int_cur = (cur_tm.tm_min + cur_tm.tm_hour*60 + cur_tm.tm_yday*60*24)/EOC_15MIN_INT_LEN;
    int shift_num = int_diff(_15m_int,_15m_int_cur,EOC_15MIN_INTS,"15m");
		// PDEBUG(DFULL,"SHIFT=%d,cur=%d,last=%d",shift_num,_15m_int_cur,_15m_int);
    if( shift_num ){
		// PDEBUG(DFULL,"----------- Shift 15MINUTES ---------------");
		update_mon_sec();
		_15min_ints.shift(shift_num);
		time(&_15m);
		get_localtime(&_15m,_15m_tm);
		_15m_tm.tm_min = (((int)_15m_tm.tm_min)/EOC_15MIN_INT_LEN)*EOC_15MIN_INT_LEN;
		_15m_tm.tm_sec = 0;
		_15min_ints[0]->tstamp = mktime(&_15m_tm);
    }

    if( _1d_tm.tm_year == cur_tm.tm_year ){
			//PDEBUG(DFULL,"----------- Shift 1DAYS ---------------");
			//PDEBUG(DFULL,"shift days: sshift_val=%d",_1d_tm.tm_yday - cur_tm.tm_yday);
    	shift_num = cur_tm.tm_yday - _1d_tm.tm_yday;

			_1d_tm = cur_tm;
			_1d_tm.tm_sec = 0;
			_1d_tm.tm_min = 0;
			_1d_tm.tm_hour = 0;
			time_t cur_ts = mktime(&_1d_tm);
			// Monitor Seconds counter update
			if( shift_num ){
				// update_mon_sec(); - dont need it, because in 15min part all work is done
				_1day_ints.shift(shift_num);
				_1day_ints[0]->tstamp = cur_ts;
			}
    }else{
        // TODO : count difference to different years
    }
}


EOC_loop::
EOC_loop() : _15min_ints(EOC_15MIN_INTS) , _1day_ints(EOC_1DAY_INTS) {
  struct tm _15mtm,_1dtm;
  time_t t;
  memset(&state,0,sizeof(state));
  state.loop_attn = -127;
  state.snr_marg = -127;
    
	is_first_msg = 1;
  memset(&last_msg,0,sizeof(last_msg));
  time(&t);
  if( get_localtime(&t,_15mtm)){
    // TODO: eoc_log("Error getting time");
    return ;
  }
  if( get_localtime(&t,_1dtm)){
    // TODO: eoc_log("Error getting time");
    return ;
  }

  _15mtm.tm_min = (((int)_15mtm.tm_min)/EOC_15MIN_INT_LEN)*EOC_15MIN_INT_LEN;
  _15mtm.tm_sec = 0;

  _1dtm.tm_sec = 0;
  _1dtm.tm_min = 0;
  _1dtm.tm_hour = 0;

  _15min_ints[0]->tstamp = mktime(&_15mtm);
  _1day_ints[0]->tstamp = mktime(&_1dtm);

	// Link UP/DOWN status
	lstate = 0;
	time(&moni_ts);

}


int EOC_loop::
short_status(s8 snr_margin)
{
  shift_rings();
  // change online data
  state.snr_marg = snr_margin;
	// Monitoring seconds
	update_mon_sec();

	PDEBUG(DFULL,"SHORT INFO: snr=%d\n",snr_margin);
}

int EOC_loop::
full_status(side_perf *info,int rel)
{
  counters_t cntrs;
	side_perf last = last_msg;
	memset(&cntrs,0,sizeof(cntrs)); /// DEBUG - TODO: delete
    PDEBUG(DINFO,"FULL STATUS RESPONSE: attn=%d SNR=%d\n",info->loop_attn,info->snr_marg);
	if( rel ){
	    cntrs.es = info->es;
    	cntrs.ses = info->ses;
	    cntrs.crc = info->crc;
    	cntrs.losws = info->losws;
	    cntrs.uas = info->uas;
	} else {
		if( is_first_msg ){
			is_first_msg = 0;
			memcpy(&last_msg,info,sizeof(side_perf));
		    state.loop_attn = info->loop_attn;
    		state.snr_marg = info->snr_marg;
			goto exit;
		}
	  status_diff(info,cntrs);
	}

  shift_rings();
  // change online data
  state.loop_attn = info->loop_attn;
  state.snr_marg = info->snr_marg;
  setup_cur_status(info);
  // Change counters
  state.elem.addit(cntrs);
	tstate.addit(cntrs);
  _15min_ints[0]->addit(cntrs);
  _1day_ints[0]->addit(cntrs);
	// Monitoring seconds
	update_mon_sec();
 exit:
    PDEBUG(DFULL,"FULL STATUS RESPONCE:");
		PDEBUG(DFULL,"info: es(%u) ses(%u) crc(%u) losws(%u) uas(%u)\n",
				info->es,info->ses,info->crc,info->losws,info->uas);
		PDEBUG(DFULL,"cntrs: es(%u) ses(%u) crc(%u) losws(%u) uas(%u)\n",
				cntrs.es,cntrs.ses,cntrs.crc,cntrs.losws,cntrs.uas);
		PDEBUG(DFULL,"last: es(%u) ses(%u) crc(%u) losws(%u) uas(%u)\n",
		   last.es,last.ses,last.crc,last.losws,last.uas);
	return 0;
}
