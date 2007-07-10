/*
 * EOC_db.h:
 *	EOC channel configuration & status storage
 */
 
#ifndef EOC_DATABASE_H
#define EOC_DATABASE_H

#include <generic/EOC_generic.h>
#include <generic/EOC_responses.h>


class EOC_unit{
    unit u;
    resp_inventory inv_info;
public:
    EOC_unit(unit u_in,resp_discovery *resp){
	u = u_in;
	inv_info = *resp;
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
    
    inline int add_unit(unit u, resp_discovery *resp){
	if( u<=unknown || u>(unit)EOC_MAX_UNITS )
	    return -1;
	if( units[(int)u-1]){
	    if( units[(int)u-1].integrity(resp) )
		return -1;
	    return 0;
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
