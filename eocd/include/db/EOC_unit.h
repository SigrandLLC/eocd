/*
 * EOC_db.h:
 *	EOC channel configuration & status storage
 */
 
#ifndef EOC_UNIT_H
#define EOC_UNIT_H

#include <generic/EOC_generic.h>
#include <generic/EOC_responses.h>
#include <db/EOC_side.h>
#define EOC_SIDES_NUM 2

class EOC_unit{
 public:
    typedef enum { span = 0, local } power_t;
 protected:
    unit u;
    u8 eoc_softw_v;
    resp_inventory inv_info;
    u8 inv_info_setted;
    resp_sensor_state sensors_cur;
    u8 sens1,sens2,sens3;
    
    power_t power;
    EOC_side *side[EOC_SIDES_NUM];
 public:
    EOC_unit(unit u_in,resp_discovery *resp,int loops){
		inv_info_setted = 0;
		eoc_softw_v = resp->eoc_softw_ver;
		for( int i=0; i<EOC_SIDES_NUM;i++)
			side[i] = NULL;
		u = u_in;
		memset(&sensors_cur,0,sizeof(sensors_cur));
		sens1 = 0;
		sens2 = 0;	
		sens3 = 0;
		switch( u ){
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
    int set_inv_info(resp_inventory *resp){
		inv_info = *resp;
		inv_info_setted = 1;
    }
    u8 eoc_softw_ver(){ return eoc_softw_v; }
    
    EOC_side *nside(){ return side[net_side]; }
    EOC_side *cside(){ return side[cust_side]; }

    int integrity(resp_inventory *resp){
		if( !inv_info_setted )
			return 0;
		if( memcmp(resp,&inv_info,sizeof(inv_info) ) )
			return -1;
		return 0;
    }
    resp_inventory inventory_info(){ return inv_info; }
    void sensor_resp(resp_sensor_state *resp){
		//	PDEBUG(DINFO,"SAVE SENSOR STATE: s1(%d), s2(%d), s3(%d)",resp->sensor1,resp->sensor2,resp->sensor3);
		sensors_cur = *resp;
		sens1 += resp->sensor1;
		sens2 += resp->sensor2;
		sens3 += resp->sensor3;
    }
	inline void link_up(){
		for(int i=0;i<EOC_SIDES_NUM;i++)
			if(side[i])
				side[i]->link_up();
	}
	inline void link_down(){
		PDEBUG(DERR,"unit link down");
		for(int i=0;i<EOC_SIDES_NUM;i++){
			if(side[i]){
				PDEBUG(DERR,"down side %d",i);
				side[i]->link_down();
				PDEBUG(DERR,"down side %d - successfully",i);
			}
		}
	}
};

#endif
 
