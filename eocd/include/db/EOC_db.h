/*
 * EOC_db.h:
 *	EOC channel configuration & status storage
 */
 
#ifndef EOC_DATABASE_H
#define EOC_DATABASE_H

#include <generic/EOC_generic.h>
#include <generic/EOC_responses.h>

#define EOC_SIDES_NUM 2
/*
class EOC_loop{
protected:
    s8 snr_marg,loop_attn;
    u8 es,ses,losw;
    u16 crc;
    

*/
class EOC_unit{
public:
//    enum Sides { net_side=0,cust_side };
protected:
    unit u;
    resp_inventory inv_info;
//    EOC_side *side[EOC_SIDES_NUM];
public:
    EOC_unit(unit u_in,resp_inventory *resp){
	u = u_in;
//	inv_info = *resp;
//	for( int i; i<EOC_SIDES_NUM;i++)
//	    side[i] = NULL;
    }
//    EOC_side *nside(){ return sides[net_side]; }
//    EOC_side *cside(){ return sides[cust_side]; }
    int integrity(resp_inventory *resp){
//	if( *resp == inv_info )
//	    return 0;
	return -1;
    }
};

class EOC_db{
    EOC_unit *units[EOC_MAX_UNITS];
public:
    EOC_db(){
	int i;
	for(i=0;i<EOC_MAX_UNITS;i++)
	    units[i] = NULL;
    }
    
    inline int add_unit(unit u, resp_inventory *resp){
	if( u<=unknown || u>(unit)EOC_MAX_UNITS )
	    return -1;
	if( units[(int)u-1]){
	    if( units[(int)u-1]->integrity(resp) ){
		for(int i = (int)u-1;i<EOC_MAX_UNITS;i++){
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
	units[(int)u-1] = new EOC_unit(u,resp);
    }
    inline int clear(){
	for(int i=0; i<EOC_MAX_UNITS;i++){
	    if( units[i] )
		delete units[i];
	}
    }
};

#endif
