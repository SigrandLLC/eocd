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
#include <app-interface/app_frame.h>

class EOC_db{
    // Poller REsponse handler prototype
    typedef int (*response_handler_t)(EOC_db *d,EOC_msg *m);
    typedef int (*app_handler_t)(EOC_db *db,app_frame *fr);

    EOC_unit *units[MAX_UNITS];
    response_handler_t handlers[RESPONSE_QUAN];
    app_handler_t app_handlers[app_frame::ids_num];
    EOC_scheduler *sch;
    u8 loop_num;

    static int _resp_discovery(EOC_db *db,EOC_msg *m);
    static int _resp_inventory(EOC_db *db,EOC_msg *m);
    static int _resp_configure(EOC_db *db,EOC_msg *m);
    static int _resp_test(EOC_db *db,EOC_msg *m);
    static int _resp_status(EOC_db *db,EOC_msg *m);
    static int _resp_nside_perf(EOC_db *db,EOC_msg *m);
    static int _resp_cside_perf(EOC_db *db,EOC_msg *m);

    static int _appreq_inventory(EOC_db *db,app_frame *fr);
    static int _appreq_endpcur(EOC_db *db,app_frame *fr);
    static int _appreq_endp15min(EOC_db *db,app_frame *fr);
    static int _appreq_endp1day(EOC_db *db,app_frame *fr);
    static int _appreq_endpmaint(EOC_db *db,app_frame *fr);
    static int _appreq_unitmaint(EOC_db *db,app_frame *fr);

    inline int register_handlers();

public:
    EOC_db(EOC_scheduler *s,int lnum);
    int response_chk(EOC_msg *m){ return 0; }
    int response(EOC_msg *m);

    // TODO: 
    // what to do if inventory information of unit differs
    int add_unit(unit u, resp_inventory *resp);
    int clear();
    int app_request(app_frame *fr);
    inline int unit_quan(){
	int i;
	for(i=0;units[i]!=NULL;i++);
	if( i<2 ) return -1;
	return i-2;
    }
    int check_exist(unit u){
	if( units[(int)u - 1] )
	    return 0;
	return -1;
    }

    int check_exist(unit u,EOC_unit::Sides s){
	if( check_exist(u) )
	    return -1;
	switch( s ){
	case EOC_unit::net_side:
	    if( !units[(int)u-1]->nside() )
		return 0;
	    return 0;
	case EOC_unit::cust_side:
	    if( !units[(int)u-1]->cside() )
		return 0;
	    return 0;
	}
	return 1;
    }

    int check_exist(unit u,EOC_unit::Sides s,int loop)
    {
	if( check_exist(u,s) )
	    return -1;
	EOC_side *side;
	switch( s ){
	case EOC_unit::net_side:
	    side = units[(int)u-1]->nside();
	    break;
	case EOC_unit::cust_side:
	    side = units[(int)u-1]->cside();
	    break;
	}
	if( side->get_loop(loop) )
	    return 0;
	return -1;
    }
};

#endif
