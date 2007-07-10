#include<malloc.h>
#include <devs/EOC_dummy1.h>
#include <engine/EOC_engine.h>


/* ENGINE TEST */
int main()
{
    unit s,d;
    char type;
    dummy_channel mr1_1,mr1_2,r1r2_1,r1r2_2,r2r3_1,r2r3_2,r3r4_1,r3r4_2,r4r5_1,r4r5_2,r5r6_1,r5r6_2,r6r7_1,r6r7_2,r7s_1,r7s_2;
    EOC_dummy1 *n1 = new EOC_dummy1(&mr1_1,&mr1_2);
    
    EOC_dummy1 *n21 = new EOC_dummy1(&mr1_2,&mr1_1);
    EOC_dummy1 *n22 = new EOC_dummy1(&r1r2_1,&r1r2_2);
    
    EOC_dummy1 *n31 = new EOC_dummy1(&r1r2_2,&r1r2_1);    
    EOC_dummy1 *n32 = new EOC_dummy1(&r2r3_1,&r2r3_2);
    
    EOC_dummy1 *n41 = new EOC_dummy1(&r2r3_2,&r2r3_1);    
    EOC_dummy1 *n42 = new EOC_dummy1(&r3r4_1,&r3r4_2);
    
    EOC_dummy1 *n51 = new EOC_dummy1(&r3r4_2,&r3r4_1);    
    EOC_dummy1 *n52 = new EOC_dummy1(&r4r5_1,&r4r5_2);
    
    EOC_dummy1 *n61 = new EOC_dummy1(&r4r5_2,&r4r5_1);    
    EOC_dummy1 *n62 = new EOC_dummy1(&r5r6_1,&r5r6_2);
    
    EOC_dummy1 *n71 = new EOC_dummy1(&r5r6_2,&r5r6_1);    
    EOC_dummy1 *n72 = new EOC_dummy1(&r6r7_1,&r6r7_2);
    
    EOC_dummy1 *n81 = new EOC_dummy1(&r6r7_2,&r6r7_1);    
    EOC_dummy1 *n82 = new EOC_dummy1(&r7s_1,&r7s_2);
    
    
    EOC_dummy1 *n9 = new EOC_dummy1(&r7s_2,&r7s_1);
    
    EOC_engine *e1 = new EOC_engine(master,(EOC_dev*)n1,"config");
    EOC_engine *e2 = new EOC_engine(repeater,(EOC_dev*)n22,(EOC_dev*)n21);    
    EOC_engine *e3 = new EOC_engine(repeater,(EOC_dev*)n32,(EOC_dev*)n31);    
    EOC_engine *e4 = new EOC_engine(repeater,(EOC_dev*)n42,(EOC_dev*)n41);    
    EOC_engine *e5 = new EOC_engine(repeater,(EOC_dev*)n52,(EOC_dev*)n51);    
    EOC_engine *e6 = new EOC_engine(repeater,(EOC_dev*)n62,(EOC_dev*)n61);    
    EOC_engine *e7 = new EOC_engine(repeater,(EOC_dev*)n72,(EOC_dev*)n71);    
    EOC_engine *e8 = new EOC_engine(repeater,(EOC_dev*)n82,(EOC_dev*)n81);    

    EOC_engine *e9 = new EOC_engine(slave,(EOC_dev*)n9,"config");    

    while(1){
	e1->schedule();
        e2->schedule();
        e3->schedule();
        e4->schedule();
        e5->schedule();
        e6->schedule();
        e7->schedule();
        e8->schedule();
        e9->schedule();
    }
    return 0;
}



/* TEST EOC_DUMMY1 DEVICE
int main()
{
    unit s,d;
    char type;
    EOC_msg *m;
    char a[256];
    int len;
    
    dummy_channel d1,d2;
    
    EOC_dummy1 *n1 = new EOC_dummy1(&d1,&d2);
    EOC_dummy1 *n2 = new EOC_dummy1(&d2,&d1);

    for(int i=0;i<10;i++){
	for(int j=0;j<i+20;j++)
	    a[j] = 'a'+i;
	m = new EOC_msg;
	m->setup(a,20+i);
	n1->send(m);
	if( m = n2->recv() ){
	    printf("%d:",i);
	    for(int k =0;k<m->msize();k++)
		printf("%c",m->mptr()[k]);
	    printf("\n");
	}

	for(int j=0;j<i+20;j++)
	    a[j] = 'z'-i;
	m = new EOC_msg;
	m->setup(a,20+i);
	n2->send(m);
	if( m = n1->recv() ){
	    printf("%d:",i);
	    for(int k =0;k<m->msize();k++)
		printf("%c",m->mptr()[k]);
	    printf("\n");
	}
    }

    return 0;
}

*/




/*
ROUTER TEST

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

*/

/*
 SCHEDULER TEST

    EOC_msg *m = new EOC_msg(100);
    m->resize(40);
    sch->link_state(EOC_dev::ONLINE);
    while( !sch->request(s,d,(unsigned char&)type) );
    m->type(type+128);
    m->dst(s);
    sch->print();
    for(int i=(int)sru1;i<(int)sru4;i++){
	m->src((unit)i);
	sch->response(m);
	sch->print();	
    }
    m->src(stu_r);
    sch->response(m);
    sch->print();
    while( 1 ){
	while( !sch->request(s,d,(unsigned char&)type) ){
    	    sch->print();	
	    m->type(type+128);
	    m->dst(s);
	    m->src(d);
	    if( sch->response(m) )
		return -1;
	}
	sleep(1);
	sch->tick();
    }        
*/