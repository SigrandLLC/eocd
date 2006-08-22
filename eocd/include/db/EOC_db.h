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
#include <app-if/app_frame.h>

class EOC_db{
    // Poller REsponse handler prototype
    typedef int (*response_handler_t)(EOC_db *d,EOC_msg *m,int check);
    typedef int (*app_handler_t)(EOC_db *db,app_frame *fr);

    EOC_unit *units[MAX_UNITS];
    u8 units_discov[MAX_UNITS];
    response_handler_t handlers[RESPONSE_QUAN];
    app_handler_t app_handlers[app_ids_num];
    EOC_scheduler *sch;
    u8 loop_num;

    static int _resp_discovery(EOC_db *db,EOC_msg *m,int check);
    static int _resp_inventory(EOC_db *db,EOC_msg *m,int check);
    static int _resp_configure(EOC_db *db,EOC_msg *m,int check);
    static int _resp_test(EOC_db *db,EOC_msg *m,int check);
    static int _resp_status(EOC_db *db,EOC_msg *m,int check);
    static int _resp_nside_perf(EOC_db *db,EOC_msg *m,int check);
    static int _resp_cside_perf(EOC_db *db,EOC_msg *m,int check);
    static int _resp_sensor_state(EOC_db *db,EOC_msg *m,int check);

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
    int response(EOC_msg *m,int check = 0);

    // TODO: 
    // what to do if inventory information of unit differs
//    int add_unit(unit u, resp_inventory *resp);
    int clear();
    int app_request(app_frame *fr);
    int unit_quan();
    int link_established();

    int check_exist(unit u);
    EOC_side *check_exist(unit u,side s);
    EOC_loop *check_exist(unit u,side s,int loop);
    void link_down();
};

#endif
