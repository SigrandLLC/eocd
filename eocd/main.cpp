#include<malloc.h>
#include "devs/EOC_sg17.h"
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
    EOC_sg17 *n1 = new EOC_sg17("file-1");
    EOC_sg17 *n21 = new EOC_sg17("file-1");
    EOC_sg17 *n22 = new EOC_sg17("file-2");
    EOC_sg17 *n3 = new EOC_sg17("file-2");    
    
    EOC_router *r1= new EOC_router(master,n1);
    EOC_router *r2= new EOC_router(repeater,n22,n21);
    EOC_router *r3= new EOC_router(slave,n3);
    r2->nsunit(sru1);

    EOC_msg *m = new EOC_msg;
    char a[]={0x13,0x12,0xaa,0xbb,0xcc,0xdd,0xff};
    char *p;
    int i=0;

while(1){    

    m = new EOC_msg;
    p = (char*)malloc(sizeof(a));
    memcpy(p,a,sizeof(a));
    m->clean();    
    p[0] = 0x13;
    m->setup(p,7);
    r1->send(m);
    delete m;
    
    while( !(m = r2->receive()) );
    printf("Repeater get:\n");
    msg_print(m);
    
    m->response(4);
    m->payload()[0]='a';
    m->payload()[1]='b';
    m->payload()[2]='c';
    m->payload()[3]='d';
    r2->send(m);
    delete m;
    
    while( !(m = r1->receive()) );
    printf("Master get:\n");
    msg_print(m);
    delete m;
    
    m = new EOC_msg;
    p = (char*)malloc(sizeof(a));
    memcpy(p,a,sizeof(a));
    m->clean();    
    p[0] = 0x12;
    m->setup(p,7);
    r1->send(m);
    delete m;
    
    for(i=0;i<2;i++){
	r2->receive();
	if( m = r3->receive() ){
	    printf("SLave get:\n");
	    msg_print(m);
	    m->response(4);
	    m->payload()[0]='r';
	    m->payload()[1]='s';
	    m->payload()[2]='t';
	    m->payload()[3]='i';
	    r2->send(m);
	    delete m;

	    while( !(m = r1->receive()) );
	    printf("Master get:\n");
	    msg_print(m);    
	    delete m;
        }

    }

}
    return 0;
}