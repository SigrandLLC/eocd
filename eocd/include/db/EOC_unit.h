/*
 * EOC_db.h:
 *  EOC channel configuration & status storage
 */

#ifndef EOC_UNIT_H
#define EOC_UNIT_H

#include <generic/EOC_generic.h>
#include <generic/EOC_responses.h>
#include <utils/EOC_ring_container.h>
#include <db/EOC_side.h>
#define EOC_SIDES_NUM 2
#define SENS_EVENTS_NUM 100

class sens_events {
private:
	time_t _start;
	time_t _end;
	int _cnt;
public:
	sens_events(){
		_start = _end = 0;
		_cnt = 0;
	}
	sens_events(time_t start){
		_start = start; 
		_end = 0;
		_cnt = 1;
	}
	
	int event_start(time_t start)
	{
		_start = start;
		_cnt++;
	}
	bool event_started(){
		return (_start) > 0 ? true : false;
	}

	void event_end(time_t end){
		_end = end;
	}

	bool event_ended(){
		return (_end) > 0 ? true : false;
	}

	bool event_colsed(){
		return (_end) > 0 ? true : false;
	}	

	bool related_event(time_t start){
		if( _end ){
			if( (start-_end) < 2*60 && (_start - _end)< 15*60 &&
				_cnt<100 )
				return true;
			else
				return false;
		}
		return true;
	}
	
	void event_add(time_t start){
		_end = 0;
		_cnt++;
	}
	int event_descr(time_t &start,time_t &end){
		start = _start;
		end = _end;
		return _cnt;
	}
};


class EOC_unit {
public:
	typedef enum {
		span = 0, local
	} power_t;
	
protected:
	unit u;
	u8 eoc_softw_v;
	resp_inventory inv_info;
	u8 inv_info_setted;
	resp_sensor_state sensors_cur;
	u8 sens1, sens2, sens3;
	EOC_ring_container<sens_events> sens_event[3];

	power_t power;
	EOC_side *side[EOC_SIDES_NUM];
public:
	EOC_unit(unit u_in, resp_discovery *resp, int loops) 
	{
		sens_event[0].resize(SENS_EVENTS_NUM);
		sens_event[1].resize(SENS_EVENTS_NUM);		
		sens_event[2].resize(SENS_EVENTS_NUM);		
		inv_info_setted = 0;
		eoc_softw_v = resp->eoc_softw_ver;
		for(int i = 0;i<EOC_SIDES_NUM;i++)
			side[i] = NULL;
		u = u_in;
		memset(&sensors_cur, 0, sizeof(sensors_cur));
		sens1 = 0;
		sens2 = 0;
		sens3 = 0;
		switch(u){
		case stu_c:
			side[cust_side] = new EOC_side(loops);
			break;
		case stu_r:
			side[net_side] = new EOC_side(loops);
			break;
		default:
			side[cust_side] = new EOC_side(loops);
			side[net_side] = new EOC_side(loops);
			break;
		}
	}
	int set_inv_info(resp_inventory *resp) {
		inv_info = *resp;
		inv_info_setted = 1;
	}

	int set_inv_info(resp_inventory_1 *resp) {
		inv_info.shdsl_ver = resp->shdsl_ver;
		//      inv_info = *resp;
		inv_info_setted = 1;
	}

	u8 eoc_softw_ver() {
		return eoc_softw_v;
	}

	EOC_side *nside() {
		return side[net_side];
	}
	EOC_side *cside() {
		return side[cust_side];
	}

	int integrity(resp_inventory *resp) {
		if(!inv_info_setted)
			return 0;
		if(memcmp(resp, &inv_info, sizeof(inv_info)))
			return -1;
		return 0;
	}

	int integrity(resp_inventory_1 *resp) {
		int ret;
int tmp_lev = debug_lev;
debug_lev = 10;
		PDEBUG(DFULL,"start");
		if(!inv_info_setted){
			PDEBUG(DFULL,"!inv_info_setted");
			ret = 0;
			goto exit;
		}
		if(inv_info.shdsl_ver!=resp->shdsl_ver){
			PDEBUG(DFULL,"inv_info.shdsl_ver!=resp->shdsl_ver");
			ret = -1;
			goto exit;
		}

		if(memcmp(inv_info.ven_lst, resp->ven_lst, 3)){
			PDEBUG(DFULL,"memcmp(inv_info.ven_lst, resp->ven_lst, 3)");
			ret = -1;
			goto exit;
		}

		if(memcmp(inv_info.ven_issue, resp->ven_issue, 2)){
			PDEBUG(DFULL,"memcmp(inv_info.ven_issue, resp->ven_issue, 2)");
			ret = -1;
			goto exit;
		}

		if(memcmp(inv_info.softw_ver, resp->softw_ver, 2)){
			PDEBUG(DFULL,"memcmp(inv_info.softw_ver, resp->softw_ver, 2)");
			ret = -1;
			goto exit;
		}
		/* TODO: complete!!!!
		 typedef struct{
		 u8 unit_id_code[10];
		 u8 res1;
		 u8 ven_id[8];
		 u8 ven_model[12];
		 u8 ven_serial[12];
		 u8 other[12];
		 */
	exit:
		debug_lev = tmp_lev;
exit(0);
		return ret;
	}

	resp_inventory inventory_info() {
		return inv_info;
	}

	// Regenerators sensors
	void sensor_resp(resp_sensor_state *resp) {
		PDEBUG(DERR, "SAVE SENSOR STATE: s1(%d), s2(%d), s3(%d)",
			resp->sensor1, resp->sensor2, resp->sensor3);
		sensors_cur = *resp;
		int events[3] = {0,0,0};
		time_t tstamp = time(NULL);
		
		sens1 += resp->sensor1;
		events[0] = resp->sensor1;
		sens2 += resp->sensor2;
		events[1] = resp->sensor2;
		sens3 += resp->sensor3;
		events[2] = resp->sensor3;

		for(int i=0; i<3;i++){
			EOC_ring_container<sens_events> &sens = sens_event[i];
			if( events[i]){
				if( !sens[0]->event_started() ){
					PDEBUG(DERR,"First event on sensor #%d",i);
					sens[0]->event_start(tstamp);
				}else{
					PDEBUG(DERR,"Non-First event on sensor #%d",i);
					if( sens[0]->related_event(tstamp) ){
						PDEBUG(DERR,"Related event on sensor #%d",i);
						sens[0]->event_add(tstamp);
					}else{
						PDEBUG(DERR,"Non-Related event on sensor #%d",i);
						sens.shift(1);
						sens[0]->event_start(tstamp);
						for(int K=0;sens[K];K++){
							printf("\nindex=%d ");
							char s[256];
							time_t start,end;
							int cnt = sens[K]->event_descr(start,end);
							strftime(s, 256, "%d %b %G", localtime(&start));
							printf("day_start=%s,",s);
							strftime(s, 256, "%R", localtime(&start));
							printf("time_start = %s,",s);
							if( !end ){
								end = time(NULL);
							}
							strftime(s, 256, "%R", localtime(&end));
							printf("time_end = %s,",s);
							
							printf("cnt=%d\n",cnt);
							
						}
					}
				}
			}else{
				if( sens[0]->event_started() && 
					!sens[0]->event_ended()){
					PDEBUG(DERR,"Close event on sensor #%d",i);
					sens[0]->event_end(tstamp);
				}
			}
		}				
	}

	inline void sensor_get(resp_sensor_state &st) {
		st = sensors_cur;
	}

	inline void sensor_get(resp_sensor_state &st, u8 &s1, u8 &s2, u8 &s3) {
		st = sensors_cur;
		s1 = sens1;
		s2 = sens2;
		s3 = sens3;
	}

    int sensor_event(u8 sens_num,u32 index,sens_events &ev){
		if( !(index < SENS_EVENTS_NUM) || !(sens_num < 3) )
			return -1;
		if( !sens_event[sens_num][index] ||
			!sens_event[sens_num][index]->event_started() )
			return -1;
		ev = *sens_event[sens_num][index];
		return 0;
    }
	
	// Link handling
	inline void link_up() {
		for(int i = 0;i<EOC_SIDES_NUM;i++){
			if(side[i]){
				PDEBUG(DFULL, "side%d = %p", i,side[i]);
				side[i]->link_up();
				PDEBUG(DFULL, "side%d - successfully", i);
			}

		}
	}
	inline void link_down() {
		PDEBUG(DFULL, "unit link down");
		for(int i = 0;i<EOC_SIDES_NUM;i++){
			if(side[i]){
				PDEBUG(DFULL, "side%d = %p", i,side[i]);
				side[i]->link_down();
				PDEBUG(DFULL, "side %d - successfully", i);
			}
		}
	}
};

#endif

