/*
 * EOC_db.cpp
 *  EOC Data base unit, provide SHDSL channel information storage and
 *  acces to it
 */

#define EOC_DEBUG
#include<eoc_debug.h>

#include <syslog.h>

#include <generic/EOC_responses.h>
#include <generic/EOC_msg.h>
#include <db/EOC_db.h>
#include <app-if/err_codes.h>

static void resp_perf_convert(side_perf *resp, char *pay, int size)
{
	int s = (long int)&resp->crc-(long int)resp-1;
	int s2;
	int i;
	PDEBUG(DFULL, "start, copy first part size = %d", s);

	//memcpy(resp,pay,s);
	for(i = 0;i<s;i++){
		*((char*)resp+i) = *(pay+i);
		PDEBUG(DFULL, "Write %02x to %d", *(pay+i), i);
	}

	//memcpy(resp+5,pay+4,6);
	s2 = sizeof(*resp)-((long int)&resp->crc-(long int)resp);
	PDEBUG(DFULL, "copy second part size = %d", s2);
	for(i = 0;i<s2;i++){
		*((char*)resp+i+s+1) = *(pay+i+s);
		PDEBUG(DFULL, "Write %02x to %d", *(pay+i+s), i+s+1);
	}

	// Change order of bytes in CRC counter
	char tmp, *val = (char*)&resp->crc;
	tmp = val[0];
	val[0] = val[1];
	val[1] = tmp;

	PDEBUG(DFULL, "end, resp start=%p, rest_end=%p", resp, resp+5+6);
}

int EOC_db::register_handlers()
{
	// register EOC responses handlers
	handlers[RESP_IND(RESP_DISCOVERY)] = _resp_discovery;
	handlers[RESP_IND(RESP_INVENTORY)] = _resp_inventory;
	handlers[RESP_IND(RESP_CONFIGURE)] = _resp_configure;
	handlers[RESP_IND(RESP_STATUS)] = _resp_status;
	handlers[RESP_IND(RESP_NSIDE_PERF)] = _resp_nside_perf;
	handlers[RESP_IND(RESP_CSIDE_PERF)] = _resp_cside_perf;
	handlers[RESP_IND(RESP_SENSOR_STATE)] = _resp_sensor_state;

	// register Application requests for info
	app_handlers[APP_INVENTORY] = _appreq_inventory;
	app_handlers[APP_ENDP_CUR] = _appreq_endpcur;
	app_handlers[APP_ENDP_15MIN] = _appreq_endp15min;
	app_handlers[APP_ENDP_1DAY] = _appreq_endp1day;
	app_handlers[APP_ENDP_MAINT] = _appreq_endpmaint;
	app_handlers[APP_UNIT_MAINT] = _appreq_unitmaint;
	app_handlers[APP_LOOP_RCNTRST] = _appreq_cntrst;
	app_handlers[APP_SENSORS] = _appreq_sensors;
	app_handlers[APP_SENSOR_FULL] = _appreq_sensor_full;
}

EOC_db::EOC_db(EOC_scheduler *s, int lnum)
{
	int i;
	for(i = 0;i<MAX_UNITS;i++){
		units[i] = NULL;
		units_discov[i] = 0;
	}

	for(i = 0;i<RESPONSE_QUAN;i++)
		handlers[i] = NULL;
	register_handlers();
	sch = s;
	loop_num = lnum;
}

int EOC_db::response(EOC_msg *m, int check)
{
	PDEBUG(DFULL, "start");
	u8 type = RESP_IND(m->type());
	PDEBUG(DFULL, "Check m");
	if(!m||!m->is_response()||!handlers[type])
		return -1;
	PDEBUG(DFULL, "return handlers[%d](this,m,check)", type);
	return handlers[type](this, m, check);
}

// TODO:
// what to do if inventory information of unit differs
/*
 int EOC_db::
 add_unit(unit u, resp_inventory *resp)
 {
 if( u<=unknown || u>(unit)MAX_UNITS )
 return -1;
 if( units[(int)u-1]){
 if( units[(int)u-1]->integrity(resp) ){
 for(int i = (int)u-1;i<MAX_UNITS;i++){
 if( units[i] ){
 delete units[i];
 units[i] = NULL;
 }
 }
 if( units[(int)stu_r-1] ){
 delete units[(int)stu_r-1];
 units[(int)stu_r-1] = NULL;
 }
 } else {
 return 0;
 }
 }
 units[(int)u-1] = new EOC_unit(u,resp,loop_num);
 }
 */

int EOC_db::clear()
{
	for(int i = 0;i<MAX_UNITS;i++){
		if(units[i])
			delete units[i];
	}
}

int EOC_db::check_exist(unit u,int ignore_discov)
{
	PDEBUG(DFULL, "start");
	u8 ind = (u8)u-1;
	PDEBUG(DFULL, "ind = %d", ind);
	if(!(ind<MAX_UNITS))
		return -1;

	PDEBUG(DFULL, "units[%d]=%p units_discov[%d] = %d", (int)u-1, units[(int)u
		-1], (int)u-1, units_discov[(int)u-1]);

	if( units[(int)u-1] ){
		if( ignore_discov ){
			// When we poll from application - give unit anyway
			return 0;
		}else if( units_discov[(int)u-1] ){
			// When we poll from engine -give unit only if it is discovered
			return 0;
		}
	}
	return -1;
}

EOC_side *EOC_db::check_exist(unit u, side s,int ignore_discov)
{
	PDEBUG(DFULL, "start");
	if(check_exist(u,ignore_discov))
		return NULL;
	PDEBUG(DFULL, "check_exist(u) == 0");
	switch(s){
	case net_side:
		PDEBUG(DFULL, "units[%d]->nside() = %p", (int)u-1,
			units[(int)u-1]->nside());
		return units[(int)u-1]->nside();
	case cust_side:
		PDEBUG(DFULL, "units[%d]->cside() = %p", (int)u-1,
			units[(int)u-1]->cside());
		return units[(int)u-1]->cside();
	}
	PDEBUG(DFULL, "return NULL");
	return NULL;
}

EOC_loop *EOC_db::check_exist(unit u, side s, int loop,int ignore_discov)
{
	PDEBUG(DFULL, "start");
	EOC_side *side = check_exist(u, s,ignore_discov);
	PDEBUG(DFULL, "side = %p", side);
	if(!side)
		return NULL;
	PDEBUG(DFULL, "return side->get_loop(%d)", loop);
	return side->get_loop(loop);
}

int EOC_db::unit_quan(int ignore_discov)
{
	int i, cnt = 0;
	if( !ignore_discov || link_state() ){
		// if channel is connected - use units_discov
		for(i = 0;i<MAX_UNITS;i++){
			if(units_discov[i])
				cnt++;
		}
	} else {
		// if channel is connected - use units existence
		for(i = 0;i<MAX_UNITS;i++){
			if(units[i])
				cnt++;
		}
	}
	return cnt;
}

int EOC_db::reg_quan(int ignore_discov)
{
	int i, cnt = 0;
	if( !ignore_discov || link_state() ){
		// if channel is connected - use units_discov
		for(i = 2;i<MAX_UNITS;i++){
			if(units_discov[i])
				cnt++;
		}
	} else {
		// if channel is connected - use units existence
		for(i = 2;i<MAX_UNITS;i++){
			if(units[i])
				cnt++;
		}
	}
	return cnt;
}

int EOC_db::link_state()
{
	if(units_discov[1]){
		return 1;
	}
	return 0;
}

void EOC_db::link_up()
{
	for(int i = 0;i<MAX_UNITS;i++){
		if(units[i])
			units[i]->link_up();
	}
}

void EOC_db::link_down()
{
	PDEBUG(DFULL, "Link down");
	for(int i = 0;i<MAX_UNITS;i++){
		units_discov[i] = 0;
		if(units[i]){
			PDEBUG(DFULL, "Down unit%d - not empty", i);
			units[i]->link_down();
		}
	}
}

//------------------- EOC responses -------------------------//

int EOC_db::_resp_discovery(EOC_db *db, EOC_msg *m, int check)
{
	ASSERT(m->type()==RESP_DISCOVERY);

	resp_discovery *resp = (resp_discovery *)m->payload();
	int ind = (int)m->src()-1;
	if(check)
		return 0;

	if(!db->units[ind]){
		PDEBUG(DERR, "Add new unit %d", ind);
		db->units[ind] = new EOC_unit(m->src(), resp, db->loop_num);
		db->units[ind]->link_up();
	}
	db->units_discov[ind] = 1;
	PDEBUG(DFULL,
		"DISCOVERY_RESP FROM(%d): hop=%d,resl=%d,vendor_id=%d,fwd_loss=%d",
		m->src(), resp->hop, resp->res1, resp->vendor_id, resp->fwd_loss);
	if(m->src()==stu_r){
		PDEBUG(DFULL, "Clean units list");
		for(int i = (int)(sru1-1);i<MAX_UNITS;i++){
			if(db->units[i]&&!db->units_discov[i]){
				PDEBUG(DERR, "Unit(%d) is not present anymore\n", i);
				delete db->units[i];
				db->units[i] = NULL;
			}
		}
	}
	return 0;
}

int EOC_db::_resp_inventory(EOC_db *db, EOC_msg *m, int check)
{
	ASSERT(m->type()==RESP_INVENTORY);
	ASSERT(m->payload_sz()==RESP_INVENTORY_SZ||m->payload_sz()
		==RESP_INVENTORY_SZ_1);

	int ind = (int)m->src()-1;
	if(!db->units[ind]){
		return -1;
	}

	if(check)
		return 0;

	int tmp_lev = debug_lev;
	debug_lev = 10;

	PDEBUG(DFULL, "INVENTORY_RESP FROM(%d)", m->src());

	if(m->payload_sz()==RESP_INVENTORY_SZ){
		resp_inventory *resp = (resp_inventory *)m->payload();
		/*
		 * TODO: Maybe there is better way.
		 * Now we rely on units_discov and do not delete unused or changed elements of channel.
		 // Check that regenerators was not changed
		 if(db->units[ind]->integrity(resp) && (unit)(ind+1) > stu_r ){
		 for(int i = ind+1;i<MAX_UNITS;i++){
		 if(db->units[i]){
		 PDEBUG(DFULL,"!!!!!\t\tDelete unit %d",i);
		 delete db->units[i];
		 db->units[i] = NULL;
		 }
		 }
		 }
		 */
		db->units[ind]->set_inv_info(resp);
	}else{
		resp_inventory_1 *resp = (resp_inventory_1 *)m->payload();
		/*
		 * TODO: Maybe there is better way.
		 * Now we rely on units_discov and do not delete unused or changed elements of channel.
		 * Another solution - clear all statistic for all units, not delete them!
		 // Check that regenerators was not changed
		 if(db->units[ind]->integrity(resp) && (unit)(ind+1) > stu_r){
		 for(int i = ind+1;i<MAX_UNITS;i++){
		 if(db->units[i]){
		 PDEBUG(DFULL,"!!!!!\t\tDelete unit %d",i);
		 delete db->units[i];
		 db->units[i] = NULL;
		 }
		 }
		 }
		 */
		db->units[ind]->set_inv_info(resp);
	}

	debug_lev = tmp_lev;

	return 0;
}

int EOC_db::_resp_configure(EOC_db *db, EOC_msg *m, int check)
{
	ASSERT(m->type()==RESP_CONFIGURE);
	if(m->payload_sz()!=RESP_CONFIGURE_SZ){
		PDEBUG(DFULL, "pay_sz=%d wait=%d", m->payload_sz(), RESP_CONFIGURE_SZ);
	}
	ASSERT(m->payload_sz()==RESP_CONFIGURE_SZ||m->payload_sz()
		==(RESP_CONFIGURE_SZ+1));

	resp_configure *resp = (resp_configure *)m->payload();
	if(check)
		return 0;

	int tmp_lev = debug_lev;
	debug_lev = 10;
	PDEBUG(DFULL, "CONFIGURE_RESP FROM(%d), loop(%d),snr(%d)", m->src(),
		resp->loop_attn, resp->snr_marg);
	debug_lev = tmp_lev;

	return 0;
}

int EOC_db::_resp_status(EOC_db *db, EOC_msg *m, int check)
{
	PDEBUG(DFULL, "start");
	ASSERT(m->type()==RESP_STATUS);
	ASSERT(m->payload_sz()==RESP_STATUS_SZ);
	ASSERT(m);
	PDEBUG(DFULL, "after asserts");
	resp_status *resp = (resp_status*)m->payload();
	int loop_id = resp->loop_id-1;
	EOC_loop *nsloop = NULL, *csloop = NULL;
	PDEBUG(DFULL, "nsloop");
	nsloop = db->check_exist(m->src(), net_side, loop_id,0);
	PDEBUG(DFULL, "csloop");
	csloop = db->check_exist(m->src(), cust_side, loop_id,0);
	PDEBUG(DFULL, "nsloop=%p,csloop=%p", nsloop, csloop);
	PDEBUG(DFULL, "switch(m->src())");
	switch(m->src()){
	case stu_c:
		if(!csloop)
			return -1;
		break;
	case stu_r:
		if(!nsloop)
			return -1;
		break;
	default:
		if(!csloop||!nsloop)
			return -1;
		break;
	}
	PDEBUG(DFULL, "if(check)");
	if(check)
		return 0;

	PDEBUG(DFULL, "STATUS RESPONSE: src(%d) dst(%d) ns_snr=%d, cs_snr=%d",
		m->src(), m->dst(), resp->ns_snr_marg, resp->cs_snr_marg);

	if(csloop){
		csloop->short_status(resp->cs_snr_marg);
	}
	if(nsloop){
		nsloop->short_status(resp->ns_snr_marg);
	}

	return 0;
}

int EOC_db::_resp_nside_perf(EOC_db *db, EOC_msg *m, int check)
{
	resp_cside_perf *resp, tmp_resp;
	ASSERT(m&&db);
	ASSERT(m->type()==RESP_NSIDE_PERF);
	int rel;

	PDEBUG(DFULL, "NET SIDE STATUS RESPONSE (start): src(%d) dst(%d)", m->src(), m->dst());


	if(m->payload_sz()==RESP_NSIDE_PERF_SZ-1){
		PDEBUG(DFULL, "Call resp_perf_convert");
		resp_perf_convert((side_perf*)&tmp_resp, m->payload(), m->payload_sz());
		resp = &tmp_resp;
		rel = 1;
	}else if(m->payload_sz()==RESP_NSIDE_PERF_SZ){
		resp = (resp_nside_perf*)m->payload();
	}else{
		PDEBUG(DFULL, "pay=%d, wait=%d", m->payload_sz(), RESP_NSIDE_PERF_SZ);
		ASSERT(m->payload_sz()==RESP_NSIDE_PERF_SZ);
	}

	int loop_id = resp->loop_id-1;
	EOC_loop *nsloop = NULL;
	/*
	 PDEBUGL(DFULL,"Message dump (loop=%d):\n",resp->loop_id);
	 for(int i=0;i<m->payload_sz();i++){
	 PDEBUGL(DFULL,"%02x ",*((char*)m->payload()+i));
	 }
	 PDEBUGL(DFULL,"\n");

	 PDEBUGL(DFULL,"Resp dump (loop=%d):\n",resp->loop_id);
	 for(int i=0;i<m->payload_sz();i++){
	 PDEBUGL(DFULL,"%02x ",*((char*)resp+i));
	 }
	 PDEBUGL(DFULL,"\n");
	 */

	if(!(nsloop = db->check_exist(m->src(), net_side, loop_id,0))){
		PDEBUG(DERR, "No nsloop on %d unit %d side", m->src(), net_side);
		return -1;
	}

	if(check){
		PDEBUG(DFULL, "Check == 0!");
		return 0;
	}

	PDEBUG(DFULL, "NET SIDE PERF STATUS RESPONSE (commit): src(%d) dst(%d)", m->src(), m->dst());

	if(nsloop){
		PDEBUG(DFULL, "Call nsloop->full_status");
		nsloop->full_status(resp, rel);
	}
	PDEBUG(DFULL, "Finish");
	return 0;
}

int EOC_db::_resp_cside_perf(EOC_db *db, EOC_msg *m, int check)
{
	resp_cside_perf *resp, tmp_resp;
	ASSERT(m&&db);
	ASSERT(m->type()==RESP_CSIDE_PERF);
	int rel = 0;

	PDEBUG(DFULL, "CUST SIDE STATUS RESPONSE (start): src(%d) dst(%d)", m->src(), m->dst());

	if(m->payload_sz()==RESP_NSIDE_PERF_SZ-1){
		PDEBUG(DFULL, "Call resp_perf_convert");
		resp_perf_convert((side_perf*)&tmp_resp, m->payload(), m->payload_sz());
		resp = &tmp_resp;
		rel = 1;
	}else if(m->payload_sz()==RESP_NSIDE_PERF_SZ){
		resp = (resp_cside_perf*)m->payload();
	}else{
		PDEBUG(DFULL, "pay=%d, wait=%d", m->payload_sz(), RESP_CSIDE_PERF_SZ);
		ASSERT(m->payload_sz()==RESP_NSIDE_PERF_SZ);
	}

	int loop_id = resp->loop_id-1;
	EOC_loop *csloop = NULL;

	if(!(csloop = db->check_exist(m->src(), cust_side, loop_id,0)))
		return -1;

	if(check)
		return 0;

	PDEBUG(DFULL, "CUST SIDE PERF STATUS RESPONSE (commit): src(%d) dst(%d)", m->src(), m->dst());

	if(csloop){
		csloop->full_status(resp, rel);
	}
	return 0;
}

int EOC_db::_resp_sensor_state(EOC_db *db, EOC_msg *m, int check)
{
	ASSERT(m->type()==RESP_SENSOR_STATE);
	ASSERT(m->payload_sz()==RESP_SENSOR_STATE_SZ);
	ASSERT(m);
	resp_sensor_state *resp = (resp_sensor_state*)m->payload();
	EOC_unit *unit = NULL;

	if(db->check_exist(m->src(),0))
		return -1;

	if(check)
		return 0;

	unit = db->units[(int)m->src()-1];
	if(unit){
		resp_sensor_state sresp;
		PDEBUG(DERR, "SENSOR STATE: src(%d): s1(%d), s2(%d), s3(%d)", m->src(),
			resp->sensor1, resp->sensor2, resp->sensor3);
		unit->sensor_get(sresp);
		// if state of one or more sensor is changed - log into syslog
		if((resp->sensor1!=sresp.sensor1)){
			if(resp->sensor1){
				syslog(LOG_NOTICE, "%s : Sensor1 ALARM!", m->get_chname());
			}else{
				syslog(LOG_NOTICE, "%s : Sensor1 ALARM CANSEL!",
					m->get_chname());
			}
		}
		if((resp->sensor2!=sresp.sensor2)){
			if(resp->sensor2){
				syslog(LOG_NOTICE, "%s : Sensor2 ALARM!", m->get_chname());
			}else{
				syslog(LOG_NOTICE, "%s : Sensor2 ALARM CANSEL!",
					m->get_chname());
			}
		}
		if((resp->sensor3!=sresp.sensor3)){
			if(resp->sensor3){
				syslog(LOG_NOTICE, "%s : Sensor3 ALARM!", m->get_chname());
			}else{
				syslog(LOG_NOTICE, "%s : Sensor3 ALARM CANSEL!",
					m->get_chname());
			}
		}
		unit->sensor_resp(resp);
	}
	return 0;
}

int EOC_db::_resp_test(EOC_db *db, EOC_msg *m, int check)
{
	if(m->type()!=15+128){
		return -1;
	}

	if(!db)
		return 0;

	resp_configure *resp = (resp_configure *)m->payload();
	PDEBUG(DINFO, "TEST_RESP FROM(%d), loop(%d),snr(%d)", m->src(),
		resp->loop_attn, resp->snr_marg);
	return 0;
}

// ------------ Application requests ------------------------------//
int EOC_db::app_request(app_frame *fr)
{
	PDEBUG(DERR, "Start. frame id=%d", fr->id());
	if( (fr->id()>= app_ids_num) || !app_handlers[fr->id()]){
		fr->negative(ERPARAM);
		PDEBUG(DERR, "No id=%d handler found", fr->id());
		return -1;
	}
	PDEBUG(DERR, "Call %d handler, APP_SENSOR_FULL=%d", fr->id(),APP_SENSOR_FULL);
	return app_handlers[fr->id()](this, fr);
}

int EOC_db::_appreq_inventory(EOC_db *db, app_frame *fr)
{
	inventory_payload *p = (inventory_payload*)fr->payload_ptr();
	PDEBUG(DINFO, "DB: Inventory app request");
	if(!p){
		fr->negative(ERPARAM);
		PDEBUG(DERR, "DB Inventory: eror !p");
		return -1;
	}
	if(db->check_exist((unit)p->unit,1)){
		PDEBUG(DERR, "DB Inventory: error check exist");
		fr->negative(ERNOELEM);
		return 0;
	}
	PDEBUG(DINFO, "DB Inventory: prisvaivanie");
	p->eoc_softw_ver = db->units[p->unit-1]->eoc_softw_ver();
	p->inv = db->units[p->unit-1]->inventory_info();
	p->region1 = 1;
	p->region0 = 1;
	PDEBUG(DINFO, "DB Inventory: success");
	return 0;
}

int EOC_db::_appreq_endpcur(EOC_db *db, app_frame *fr)
{
	endp_cur_payload *p = (endp_cur_payload*)fr->payload_ptr();
	counters_elem elem;
	time_t cur;
	EOC_loop *loop;

	PDEBUG(DINFO, "DB: Endpoint current app request");
	if(!p){
		fr->negative(ERPARAM);
		PDEBUG(DERR, "DB Endp cur: eror !p");
		return -1;
	}
	if(!(loop = db->check_exist((unit)p->unit, (side)p->side, p->loop,1))){
		PDEBUG(DERR,
			"DB Endp cur: error check exist: unit(%d) side(%d) loop(%d)",
			p->unit, p->side, p->loop);
		fr->negative(ERNOELEM);
		return 0;
	}
	PDEBUG(DINFO, "DB Endp cur: form response");
	if(time(&cur)<0){
		fr->negative(ERUNEXP);
		return 0;
	}

	p->cur_attn = loop->cur_attn();
	p->cur_snr = loop->cur_snr();
	p->total = loop->cur_counters();
	p->cur_status = loop->cur_status();

	// Relative counters
	p->relative = loop->cur_tcounters();
	p->relative_ts = 0;
	PDEBUG(DERR, "RELATIVE_TS1=%d, LOOP=%d", p->relative_ts,
		loop->cur_ttstamp());
	p->relative_ts = loop->cur_ttstamp();
	PDEBUG(DERR, "RELATIVE_TS2=%d", p->relative_ts);

	// Current 15-min interval
	loop->m15_counters(0, elem);
	p->cur15min = elem.cntrs;
	p->cur_15m_elaps = cur-elem.tstamp;
	// Current 1Day interval
	loop->d1_counters(0, elem);
	p->cur1day = elem.cntrs;
	p->cur_1d_elaps = cur-elem.tstamp;
	PDEBUG(DERR, "RELATIVE_TS3=%d", p->relative_ts);
	return 0;
}

int EOC_db::_appreq_endp15min(EOC_db *db, app_frame *fr)
{
	endp_15min_payload *p = (endp_15min_payload*)fr->payload_ptr();
	EOC_loop *loop;
	counters_elem elem;

	PDEBUG(DINFO, "DB: Endpoint 15 min app request");
	if(!p){
		fr->negative(ERPARAM);
		PDEBUG(DERR, "DB Endp 15min: eror !p");
		return -1;
	}
	if(!(loop = db->check_exist((unit)p->unit, (side)p->side, p->loop,1))){
		PDEBUG(DERR, "DB Endp 15 min: error check exist");
		fr->negative(ERNOELEM);
		return 0;
	}
	PDEBUG(DINFO, "DB Endp 15min: form response");

	switch(fr->type()){
	case APP_GET:
		if(loop->m15_counters(p->int_num, elem)){
			fr->negative(ERNOELEM);
			return 0;
		}
		p->cntrs = elem.cntrs;
		return 0;
	case APP_GET_NEXT: {
		PDEBUG(DERR, "Get nonzero interval");
		int int_num = p->int_num-1;
		int flag = 0;
		do{
			int_num++;
			PDEBUG(DERR, "Check %d interval", int_num);
			if(loop->m15_nx_counters(int_num, elem)){
				PDEBUG(DERR, "Error checking %d interval", int_num);
				fr->negative(ERNOELEM);
				return 0;
			}
			flag = elem.cntrs.es+elem.cntrs.ses+elem.cntrs.crc+elem.cntrs.losws
				+elem.cntrs.uas;
			PDEBUG(DERR, "Interval %d exist, flag=%d", int_num, flag);
		}while(!flag);
		p->cntrs = elem.cntrs;
		p->int_num = int_num;
		return 0;
	}
	default:
		fr->negative(ERTYPE);
		return 0;
	}
}

int EOC_db::_appreq_endp1day(EOC_db *db, app_frame *fr)
{
	endp_1day_payload *p = (endp_1day_payload*)fr->payload_ptr();
	EOC_loop *loop;
	counters_elem elem;

	PDEBUG(DINFO, "DB: Endpoint 1 day app request");
	if(!p){
		fr->negative(ERPARAM);
		PDEBUG(DERR, "DB Endp 1 day: eror !p");
		return -1;
	}
	if(!(loop = db->check_exist((unit)p->unit, (side)p->side, p->loop,1))){
		PDEBUG(DERR, "DB Endp 1 day: error check exist");
		fr->negative(ERNOELEM);
		return 0;
	}
	PDEBUG(DINFO, "DB Endp 1 day: form response");

	switch(fr->type()){
	case APP_GET:
		if(loop->d1_counters(p->int_num, elem)){
			fr->negative(ERNOELEM);
			return 0;
		}
		p->cntrs = elem.cntrs;
		return 0;
	case APP_GET_NEXT: {
		int int_num = p->int_num-1;
		int flag = 0;
		PDEBUG(DFULL, "Get-NEXT");
		do{
			int_num++;
			PDEBUG(DFULL, "Process int#%d", int_num);
			if(loop->d1_nx_counters(int_num, elem)){
				fr->negative(ERNOELEM);
				PDEBUG(DERR, "Fail at int#%d", int_num);
				return 0;
			}
			flag = elem.cntrs.es+elem.cntrs.ses+elem.cntrs.crc+elem.cntrs.losws
				+elem.cntrs.uas;
			PDEBUG(DFULL, "Int#%d -ok, flag=%d", int_num, flag);
		}while(!flag);
		p->cntrs = elem.cntrs;
		p->int_num = int_num;
		return 0;
	}
	default:
		fr->negative(ERTYPE);
		return 0;
	}
}

int EOC_db::_appreq_endpmaint(EOC_db *db, app_frame *fr)
{
	fr->negative();
	return 0;
}

int EOC_db::_appreq_unitmaint(EOC_db *db, app_frame *fr)
{
	fr->negative();
	return 0;
}

int EOC_db::_appreq_cntrst(EOC_db *db, app_frame *fr)
{
	loop_rcntrst_payload *p = (loop_rcntrst_payload*)fr->payload_ptr();
	EOC_loop *l;

	PDEBUG(DFULL, "DB: Endpoint counters reset");
	if(!p){
		fr->negative(ERPARAM);
		PDEBUG(DERR, "Error !p");
		return -1;
	}
	if(!(l = db->check_exist((unit)p->unit, (side)p->side, p->loop,1))){
		PDEBUG(DFULL,
			"DB Endpoint reset counters: error check exist: unit(%d) side(%d)",
			p->unit, p->side);
		fr->negative(ERNOELEM);
		return 0;
	}
	PDEBUG(DFULL, "DB Endpoint reset counters: form response");
	l->reset_tcounters();
	return 0;
}

int EOC_db::_appreq_sensors(EOC_db *db, app_frame *fr)
{
	PDEBUG(DERR, "start");
	sensors_payload *p = (sensors_payload*)fr->payload_ptr();
	unit u = (unit)p->unit;
	EOC_unit *un;

	PDEBUG(DFULL, "DB: Sensor state request");
	if(!p){
		fr->negative(ERPARAM);
		PDEBUG(DERR, "Error !p");
		return -1;
	}
	PDEBUG(DFULL, "#1");
	if(db->check_exist((unit)p->unit,1)){
		PDEBUG(DFULL, "DB sensor state: error check exist: unit(%d)", p->unit);
		fr->negative(ERNOELEM);
		return 0;
	}
	PDEBUG(DFULL, "#2");
	// Sensors are installed only on regenerators
	if(u==stu_c||u==stu_r){
		PDEBUG(DFULL, "DB sensor state: no sensors on terminals");
		fr->negative(ERNOELEM);
		return 0;
	}

	PDEBUG(DFULL, "Point to requested unit");
	un = db->units[(int)u-1];
	PDEBUG(DFULL, "Get info from %p", un);
	un->sensor_get(p->state, p->sens1, p->sens2, p->sens3);
	PDEBUG(DFULL, "Return successfull");
	return 0;
}

int EOC_db::_appreq_sensor_full(EOC_db *db, app_frame *fr)
{
	PDEBUG(DERR, "start");
	sensor_full_payload *p = (sensor_full_payload*)fr->payload_ptr();
	unit u = (unit)p->unit;
	EOC_unit *un;

	
	
	PDEBUG(DFULL, "DB: Sensor state request");
	if(!p){
		fr->negative(ERPARAM);
		PDEBUG(DERR, "Error !p");
		return -1;
	}
	PDEBUG(DFULL, "#1");
	if(db->check_exist((unit)p->unit,1)){
		PDEBUG(DERR, "DB sensor state: error check exist: unit(%d)", p->unit);
		fr->negative(ERNOELEM);
		return 0;
	}

	// Sensors are installed only on regenerators
	if(u==stu_c||u==stu_r){
		PDEBUG(DERR, "DB sensor state: no sensors on terminals");
		fr->negative(ERNOELEM);
		return 0;
	}
	
	if( p->num > 3 ){
		PDEBUG(DERR, "DB full sensor state: sensor #%d not exist",p->num);
		fr->negative(ERNOSENSOR);
		return 0;
	}

	PDEBUG(DFULL, "Point to requested unit");
	un = db->units[(int)u-1];
	
	PDEBUG(DERR,"num=%d, index=%d",p->num,p->index);
	
	sens_events ev;
	switch(fr->type()){
	case APP_GET:
	case APP_GET_NEXT: {
		int index = p->index;
		int flag = 0;
		PDEBUG(DFULL, "Get-NEXT");
		int i=0,ret = 0;
		
		do{
			PDEBUG(DFULL, "Process event #%d", index);
			if( ret = un->sensor_event(p->num,index,ev)){
				break;
			}
			p->ev[i].index = index; 
			p->ev[i].cnt = ev.event_descr(p->ev[i].start,p->ev[i].end);
			i++;
			index++;
		}while( i<MSG_SENS_EVENTS && index < SENS_EVENTS_NUM);

		p->last = (ret) ? 1 : 0;
		p->cnt = i;
		PDEBUG(DERR,"Return: cnt=%d, last=%d",p->cnt,p->last);
		return 0;
	}
	default:
		fr->negative(ERTYPE);
		return 0;
	}
}
