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
    
    power_t power;
    EOC_side *side[EOC_SIDES_NUM];
public:
    EOC_unit(unit u_in,resp_discovery *resp,int loops){
	inv_info_setted = 0;
	eoc_softw_v = resp->eoc_softw_ver;
	for( int i=0; i<EOC_SIDES_NUM;i++)
	    side[i] = NULL;
	u = u_in;
	switch( u ){
	case stu_c:
	    side[net_side] = new EOC_side(loops); 
	    break;
	case stu_r:
	    side[cust_side] = new EOC_side(loops);
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
    
};

#endif
 
