#include <malloc.h>
#include <generic/EOC_types.h>
#include <devs/EOC_dummy1.h>


EOC_dummy1::EOC_dummy1(char *_name, dummy_channel *send,dummy_channel *recv)
{
    snd = send;
    rcv = recv;
    memset(&perf,0,sizeof(perf));
    time(&start_ts);
    time(&last_ts);
    strcpy(name,_name);
}

int
EOC_dummy1::send(EOC_msg *m)
{
//    printf("DUMMY1: Sending\n");
    return snd->enqueue(m->mptr(),m->msize());
    return 0;
}
    
    
EOC_msg *
EOC_dummy1::recv()
{
    char *ptr,buf[256];
    int len;
//    printf("DUMMY1: Receiving\n");
    if( rcv->dequeue(buf,&len) )
	return NULL;    
    ptr = (char*)malloc(len);
    memcpy(ptr,buf,len);
    EOC_msg *msg = new EOC_msg;
    if( msg->setup(ptr,len) ){
	free(ptr);
	return NULL;
    }
    return msg;
}

EOC_dev::Linkstate
EOC_dummy1::link_state(){ return ONLINE; }

int EOC_dummy1:: 
statistics(int loop, side_perf &p)
{
    int ret = 0;
    time_t cur;
    time(&cur);
    time_t last_min_ts = (last_ts/60)*60;
    if( cur-last_min_ts > 60 ){
	int addit = (cur - last_ts)/60 + (((cur - last_ts)%60)? 1:0);
        perf.es += addit*10;
	perf.ses += addit*40;
        perf.crc += addit*70;
	ret = 1;
    }

    time_t last_15sec_ts = (last_ts/15)*15;
    if( cur - last_15sec_ts > 15 ){
	int addit = (cur - last_ts)/15 + (((cur - last_ts)%15)? 1:0);
	perf.losws += addit*10;
	ret = 1;
    }
    p = perf;
    last_ts = cur;
    return ret;
}
