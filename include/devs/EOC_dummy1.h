#ifndef EOC_DUMMY_1_H
#define EOC_DUMMY_1_H
#include<string.h>
#include<stdio.h>
#include<sys/time.h>
#include<time.h>

#include <devs/EOC_dev_terminal.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_responses.h>

class dummy_channel{
protected:
    struct{
	char buf[256];
	int len;
    } chan[256];
    int head,tail;
    inline int inc(int ind){
	if( ind+1 < 256 )
	    return ind+1;
	return 0;
    }
public:
    dummy_channel(){
	head = 0;
	tail = 0;
    }
    inline int enqueue(char *m,int col){
	if( inc(tail) == head )
	    return -1;
	memcpy(chan[tail].buf,m,col);
	chan[tail].len = col;
	tail = inc(tail);
	return 0;
    }

    inline int dequeue(char *m,int *col){
	if( head == tail )
	    return -1;
	memcpy(m,chan[head].buf,chan[head].len);
	*col = chan[head].len;
	head = inc(head);
	return 0;
    }


};


class EOC_dummy1 : public EOC_dev_terminal{
protected:
    char name[256];
    dummy_channel *snd,*rcv;
    int valid;
    time_t start_ts, last_ts;
    side_perf perf;
public:
    EOC_dummy1(char *name,dummy_channel *snd,dummy_channel *rcv);
    ~EOC_dummy1();

    int send(EOC_msg *m);
    EOC_msg *recv();

    Linkstate link_state();

    span_conf_profile_t *cur_config() { return NULL; }
    int configure(span_conf_profile_t &cfg){ return 0; }
    int configure(){ return 0; }


    int setup_current_stat(side_perf p){
	return 0;
    }

    side_perf get_current_stat(){
	return perf;
    }

    int loops() { return 1;};

    int tresholds(s8 loopattn,s8 snr){
	printf("%s: loop_attn(%d) snr_marg(%d)\n",name,loopattn,snr);
    }
    int statistics(int loop, side_perf &perf);

};

#endif



/*
int main()
{
    char a[256];
    int b;
    dummy_channel d;
    d.enqueue("aaaaaaaaaa",10);
    d.enqueue("aaaaaaaaaaa",11);
    d.enqueue("aaaaaaaaaaaa",12);
    d.enqueue("aaaaaaaaaaaaaaa",15);
    d.enqueue("aaaaa",5);
    d.enqueue("aaaaaa",6);
    d.enqueue("aaaaaaaaaa",10);
    d.enqueue("aaaaaaaaaaa",11);

    while( !d.dequeue(a,&b) ){
	a[b] = 0;
	printf("(%d): %s\n",b,a);
    }
}
*/
