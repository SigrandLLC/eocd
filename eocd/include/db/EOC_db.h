/*
 * EOC_db.h:
 *	EOC channel configuration & status storage
 */
 
#ifndef EOC_DATABASE_H
#define EOC_DATABASE_H

#include <generic/EOC_generic.h>
#include <generic/EOC_responses.h>
#include <generic/EOC_msg.h>
#include <engine/EOC_scheduler.h>
#include <db/EOC_unit.h>


class EOC_db{
    // Poller REsponse handler prototype
    typedef int (*response_handler_t)(EOC_db *d,EOC_msg *m);
    EOC_unit *units[MAX_UNITS];
    response_handler_t handlers[RESPONSE_QUAN];
    EOC_scheduler *sch;
    u8 loop_num;

    static int _resp_discovery(EOC_db *db,EOC_msg *m);
    static int _resp_inventory(EOC_db *db,EOC_msg *m);
    static int _resp_configure(EOC_db *db,EOC_msg *m);
    static int _resp_test(EOC_db *db,EOC_msg *m);
    static int _resp_status(EOC_db *db,EOC_msg *m);
    static int _resp_nside_perf(EOC_db *db,EOC_msg *m);
    static int _resp_cside_perf(EOC_db *db,EOC_msg *m);

    inline int register_handlers(){
	handlers[RESP_IND(RESP_DISCOVERY)] = _resp_discovery;
	handlers[RESP_IND(RESP_INVENTORY)] = _resp_inventory;
	handlers[RESP_IND(RESP_CONFIGURE)] = _resp_configure;
	handlers[RESP_IND(RESP_STATUS)] = _resp_status;
	handlers[RESP_IND(RESP_NSIDE_PERF)] = _resp_nside_perf;
	handlers[RESP_IND(RESP_CSIDE_PERF)] = _resp_nside_perf;
//	handlers[RESP_DISCOVERY] = _resp_discovery;
    }
public:
    EOC_db(EOC_scheduler *s,int lnum){
	int i;
	for(i=0;i<MAX_UNITS;i++)
	    units[i] = NULL;
	for(i=0;i<RESPONSE_QUAN;i++)
	    handlers[i] = NULL;
	register_handlers();
	sch = s;
	loop_num = lnum;
    }
    
    int response_chk(EOC_msg *m){ return 0; }
    
    int response(EOC_msg *m){
	u8 type = RESP_IND(m->type());
	if( !m || !m->is_response() || !handlers[type] )
	    return -1;
	return handlers[type](this,m);
    }

    // TODO: 
    // what to do if inventory information of unit differs
    inline int add_unit(unit u, resp_inventory *resp)
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
    
    inline int clear(){
	for(int i=0; i<MAX_UNITS;i++){
	    if( units[i] )
		delete units[i];
	}
    }

};

#endif
