#ifndef EOC_DUMMY_1_H
#define EOC_DUMMY_1_H
#include<string.h>
#include<stdio.h>

#include <devs/EOC_dev.h>
#include <generic/EOC_msg.h>

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
    u8 loops(){ return 1;}
    u8 perf_change(int loop){ return 1; }
    int snr(int loop) { return -10; }
};


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


class EOC_dummy1 : public EOC_dev{
protected:
    dummy_channel *snd,*rcv;
    int valid;
    u8 loop_attn_atr,snr_marg_atr;
public:
    EOC_dummy1(dummy_channel *snd,dummy_channel *rcv);
    ~EOC_dummy1();    

    int send(EOC_msg *m);
    EOC_msg *recv();

    Linkstate link_state();
    int tresholds(u8 lattn,u8 snr){
	loop_attn_atr = lattn;
	snr_marg_atr = snr;
    }
    u8 loop_attn(){ return loop_attn_atr; }
    u8 snr_marg(){return snr_marg_atr; } 
};

#endif
