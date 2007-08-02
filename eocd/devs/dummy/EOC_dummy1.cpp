#include <malloc.h>
#include <generic/EOC_types.h>
#include <devs/EOC_dummy1.h>

EOC_dummy1::EOC_dummy1(char *_name, dummy_channel *send,dummy_channel *recv)
{
    snd = send;
    rcv = recv;
    memset(&perf,0,sizeof(perf));
    perf_changed = 0;
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
