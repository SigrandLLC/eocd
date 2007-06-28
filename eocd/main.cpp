#include<malloc.h>
#include "devs/EOC_dummy.h"
#include "engine/EOC_router.h"

void msg_print(EOC_msg *m)
{
    int i;
    for(i=0;i<m->msize();i++)
        printf("%02x ",m->mptr()[i]&0xff);
    printf("\n");
}

int main()
{
    EOC_dummy *n1 = new EOC_dummy("channels/m-r1","channels/r1-m");
    EOC_dummy *n21 = new EOC_dummy("channels/r1-m","channels/m-r1");
    EOC_dummy *n22 = new EOC_dummy("channels/r1-s","channels/s-r1");
    EOC_dummy *n3 = new EOC_dummy("channels/s-r1","channels/r1-s");    
    
    EOC_router *r1= new EOC_router(master,n1);
    EOC_router *r2= new EOC_router(repeater,n22,n21);
    EOC_router *r3= new EOC_router(slave,n3);

    EOC_msg *m = new EOC_msg;
    char a[]={0x10,0x1,0};
    char *p;
    int i=0;

    m = new EOC_msg;
    p = (char*)malloc(sizeof(a));
    memcpy(p,a,sizeof(a));
    m->setup(p,sizeof(a));
    r1->send(m);
    delete m;
    
    r2->receive();
    r2->receive();
    r3->receive();    
    m = r1->receive();    
    r2->receive();    
    m = r1->receive();    

    return 0;
}