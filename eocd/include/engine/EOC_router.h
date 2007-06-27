#ifndef SIGRAND_EOC_ROUTER_H
#define SIGRAND_EOC_ROUTER_H

#include <generic/EOC_msg.h>
#include <generic/EOC_generic.h>
#include <devs/EOC_dev.h>

/*
    EOC_router:
	Provide message receiving/sending/routing based on their
	address fields in EOC message as described in G.shdsl standard
*/

class EOC_router{
public:
    struct interface{
	EOC_dev *sdev;
	unit sunit;
	EOC_msg::Direction in_dir,out_dir;
    };
    enum {SHDSL_MAX_IF=2};
    enum {CS_IND=0,NS_IND=1};    
protected:
    dev_type type;
    struct interface ifs[SHDSL_MAX_IF];
    unsigned char if_cnt,if_poll;
    int max_recv_msg;
    
    inline void zero_init();
    inline EOC_dev *get_route_dev();
    inline int out_direction(EOC_msg::Direction *dir);
    
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

inline void
EOC_router::zero_init(){
    int i;
    // initial initialising
    for(i=0;i<SHDSL_MAX_IF;i++){
        ifs[i].sdev = NULL;
        ifs[i].sunit = unknown;
	ifs[i].in_dir = ifs[i].out_dir = EOC_msg::UNDEFINED;
    }
    if_cnt = 0;
}

inline EOC_dev *
EOC_router::get_route_dev(){
    max_recv_msg = 5;
    if( type != repeater || !if_cnt )
        return NULL;
    int ind = (if_poll+1)<if_cnt ? if_poll+1 : 0;
    return ifs[ind].sdev;
}

inline int
EOC_router::out_direction(EOC_msg::Direction *dir){
    if( !if_cnt || (type==repeater) )
	return -1;
    switch(type){
    case master:
	*dir = EOC_msg::DOWNSTREAM;
	return 0;
    case slave:
	*dir = EOC_msg::UPSTREAM;
	return 0;
    }
    return -1;
}

#endif
