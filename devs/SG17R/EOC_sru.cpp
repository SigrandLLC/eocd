/*
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned short u32;
typedef signed char s8;
typedef signed short s16;
typedef signed short s32;
*/
#include <stdlib.h>

#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <devs/EOC_sru.h>


EOC_sru::EOC_sru(struct sdfe4 *h,int c){
    hwdev = h;
    ch = c;
}

int
EOC_sru::recv(EOC_msg *m){
    char *ptr;
    int size;

    // get current message
    if( (size = sdfe4_eoc_rx(&hwdev->ch[ch],&ptr)) > 0 ){
	 	return  m->setup(ptr,size);
    }
    return -1;
}

int
EOC_sru::send(EOC_msg *m){
    return sdfe4_eoc_tx(hwdev,ch,m->mptr(),m->msize());
}

int EOC_sru::
loops(){
    return 1;
}

Linkstate EOC_sru::
link_state(){
    // TODO: real status
    return 1;

}

int EOC_sru::
tresholds(s8 loop_attn,s8 snr_m)
{
    return 0;
}

int EOC_sru::
statistics(int loop,side_perf &stat)
{
    // TODO: get statistic
    return 0
}



// C-wrappers

extern "C" EOC_sru* init_SRU(struct sdfe4 *hwdev,int ch)
{
	EOC_sru *s = new EOC_sru(hwdev,ch);
	return s;
}

extern "C" int EOC_sru_send(EOC_sru *u,EOC_msg *m)
{
	return u->send(m);
}

extern "C" int EOC_sru_recv(EOC_sru *u,EOC_msg **m)
{
  	*m = new EOC_msg;
	return u->recv(*m);
}
