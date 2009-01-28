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
#define EOC_DEBUG
#include <eoc_debug.h>


class EOC_router{
 public:
    struct interface{
		EOC_dev *sdev;
		unit sunit;
		EOC_msg::Direction in_dir,out_dir;
		shdsl_state state;
    };
    enum {SHDSL_MAX_IF=2};
 protected:
    dev_type type;
    struct interface ifs[SHDSL_MAX_IF];
    unsigned char if_cnt,if_poll;
    int max_recv_msg;

    inline void zero_init();
    inline EOC_dev *get_route_dev(int if_ind);
    inline int out_direction(EOC_msg::Direction *dir);
    inline EOC_msg *process_discovery(int if_ind,EOC_msg *m);


    // ------------ loopback ------------------//
#define LOOPB_BUF_SZ 16
    int loop_head,loop_tail;
    EOC_msg *loopb[LOOPB_BUF_SZ];
    inline int inc(int ind,int max_ind){ return (ind+1<max_ind) ? ind+1 : 0; }
    inline int add_loop(EOC_msg *m){
		PDEBUG(DERR,"START: loop_head=%d,loop_tail=%d",loop_head,loop_tail);
		if( loop_head == inc(loop_tail,LOOPB_BUF_SZ) ){
			PDEBUG(DERR,"Loop is full - drom msg: src=%d, dst=%d, type=%d",m->src(),m->dst(),m->type());
			return -1;
		}
		loopb[loop_tail] = m;
		loop_tail = inc(loop_tail,LOOPB_BUF_SZ);
		return 0;
    }

    inline EOC_msg *get_loop(){
		EOC_msg *m;
		PDEBUG(DERR,"START: loop_head=%d,loop_tail=%d",loop_head,loop_tail);
		if( loop_head == loop_tail ) {
			PDEBUG(DERR,"Loop is empty - nothing to get");
			return NULL;
		}

		int tmp = loop_head;
		while( tmp != loop_tail ){
			EOC_msg *tm = loopb[tmp];
			PDEBUG(DERR,"LOOPBACK IN: src(%d) dst(%d) id(%d)",tm->src(),tm->dst(),tm->type());
			tmp = inc(tmp,LOOPB_BUF_SZ);
		}

		m = loopb[loop_head];
		loop_head = inc(loop_head,LOOPB_BUF_SZ);

		PDEBUG(DERR,"END: loop_head=%d,loop_tail=%d",loop_head,loop_tail);
		return m;
    }
	inline void free_loop(){
		while( loop_head != loop_tail ) {
			delete loopb[loop_head];
			loop_head = inc(loop_head,LOOPB_BUF_SZ);
		}
		loop_head = loop_tail = 0;
	}

    // ------------ loopback ------------------//

 public:

    EOC_router(dev_type r,EOC_dev *side);
    EOC_router(dev_type r,EOC_dev *nside,EOC_dev *cside);
    ~EOC_router();

    EOC_msg *receive();
    int send(EOC_msg *m);

    unit csunit();
    unit nsunit();
    EOC_dev *csdev();
    EOC_dev *nsdev();

    int csunit(unit u);
    int nsunit(unit u);
    int term_unit(unit u);
    void update_state();
    int loops();
};

#endif
