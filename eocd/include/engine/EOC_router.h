/*
 *    EOC_router:
 *	Provide message receiving/sending/routing based on their
 *	address fields in EOC message as described in G.shdsl standard
 */

#ifndef SIGRAND_EOC_ROUTER_H
#define SIGRAND_EOC_ROUTER_H

#include <generic/EOC_msg.h>
#include <generic/EOC_generic.h>
#include <devs/EOC_dev.h>


class EOC_router{
public:
    struct interface{
	EOC_dev *sdev;
	unit sunit;
	EOC_msg::Direction in_dir,out_dir;
	shdsl_state state;
    };
    enum {SHDSL_MAX_IF=2};
    enum {CS_IND=0,NS_IND=1};    
protected:
    dev_type type;
    struct interface ifs[SHDSL_MAX_IF];
    unsigned char if_cnt,if_poll;
    int max_recv_msg;
    
    inline void zero_init();
    inline EOC_dev *get_route_dev(int if_ind);
    inline int out_direction(EOC_msg::Direction *dir);
    inline void update_state(struct interface *);    
    inline EOC_msg *process_discovery(int if_ind,EOC_msg *m);
    
public:
    EOC_router(dev_type r,EOC_dev *side);
    EOC_router(dev_type r,EOC_dev *nside,EOC_dev *cside);
    ~EOC_router();

    EOC_msg *receive();
    int send(EOC_msg *m);

    unit csunit();
    unit nsunit();    
    int csunit(unit u);
    int nsunit(unit u);
    int term_unit(unit u);

};

#endif
