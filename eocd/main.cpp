#include<malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <devs/EOC_dummy1.h>
#include <db/EOC_loop.h>
#include <utils/EOC_ring_container.h>
#include <db/EOC_db.h>
#include <engine/EOC_engine.h>
#include <EOC_main.h>

/*
typedef struct{
    u8 :1;
    u8 losws_alarm :1;
    u8 loop_attn_alarm:1;
    u8 snr_marg_alarm :1;
    u8 dc_cont_flt:1;
    u8 dev_flt:1;
    u8 pwr_bckoff_st:1;
    u8 :1;
    s8 snr_marg;
    s8 loop_attn;
    u8 es;
    u8 ses;
    u16 crc;
    u8 losws;
    u8 uas;
    u8 pwr_bckoff_base_val:4;
    u8 cntr_rst_scur:1;
    u8 cntr_ovfl_stur:1;
    u8 cntr_rst_scuc:1;
    u8 cntr_ovfl_stuc:1;
    u8 loop_id:3;
    u8 :4;
    u8 pwr_bkf_ext:1;
}
side_perf;
*/
/*
int main()
{
    EOC_db db(;
    side_perf perf;
    memset(&perf,0,sizeof(perf));

    for(int i=0;i<1000;i++){
	perf.es += 20;
        l.full_status(&perf);
	sleep(2);
    }
    l.print_15m();    
    
    return 0;
}
*/

/* MAIN TEST */  


EOC_dummy1 *n1;
EOC_dummy1 *n21;
EOC_dummy1 *n22;
EOC_dummy1 *n3;


EOC_dev_master *
init_dev(char *name_1)
{
    if( !strcmp(name_1,"dsl0") ){
	return n1;
    }	
    if( !strcmp(name_1,"dsl1") ){
	return n3;
    }
    return NULL;
}

int main()
{
    unit s,d;
    char type;
    dummy_channel mr1_1,mr1_2,r1s_1,r1s_2;
    
    n1 = new EOC_dummy1("m-ns",&mr1_1,&mr1_2);
    n21 = new EOC_dummy1("r1-cs",&mr1_2,&mr1_1);
    n22 = new EOC_dummy1("r1-ns",&r1s_1,&r1s_2);
    n3 = new EOC_dummy1("s-cs",&r1s_2,&r1s_1);



    EOC_main m("eocd.cfg");    
    EOC_engine *e2 = new EOC_engine(n22,n21); 
    
    int k=0;
    side_perf S;
    while(k<500){
//	sleep(1);
	m.poll_channels();
        e2->schedule();
	k++;
    }

/*

int k=0;
    side_perf S;
    while(k<500){
//	sleep(1);
	e1->schedule();
        e2->schedule();
        e3->schedule();
	if( k>20 ){
		S = n1->get_current_stat();
		S.ses++;
		n1->setup_current_stat(S);


		S = n22->get_current_stat();
		S.es++;
		S.losws++;
		n22->setup_current_stat(S);

		S = n21->get_current_stat();
		S.crc++;
		S.uas++;
		n21->setup_current_stat(S);

		S = n3->get_current_stat();
		S.es++;
		n3->setup_current_stat(S);
	}
	if( k== 21 )
	    printf("---------------- Start errors ---------------------------\n");
	k++;
    }
*/    
    return 0;
}



/* ENGINE SHORT TEST   
int main()
{
    unit s,d;
    char type;
    dummy_channel mr1_1,mr1_2,r1s_1,r1s_2;
    EOC_dummy1 *n1 = new EOC_dummy1("m-ns",&mr1_1,&mr1_2);
    
    EOC_dummy1 *n21 = new EOC_dummy1("r1-cs",&mr1_2,&mr1_1);
    EOC_dummy1 *n22 = new EOC_dummy1("r1-ns",&r1s_1,&r1s_2);
    
    EOC_dummy1 *n3 = new EOC_dummy1("s-cs",&r1s_2,&r1s_1);
    
    EOC_engine *e1 = new EOC_engine((EOC_dev_master*)n1,3);
    EOC_engine *e2 = new EOC_engine((EOC_dev*)n22,(EOC_dev*)n21);    
    EOC_engine *e3 = new EOC_engine((EOC_dev*)n3);    

int k=0;
    side_perf S;
    while(k<500){
//	sleep(1);
	e1->schedule();
        e2->schedule();
        e3->schedule();
	if( k>20 ){
		S = n1->get_current_stat();
		S.ses++;
		n1->setup_current_stat(S);


		S = n22->get_current_stat();
		S.es++;
		S.losws++;
		n22->setup_current_stat(S);

		S = n21->get_current_stat();
		S.crc++;
		S.uas++;
		n21->setup_current_stat(S);

		S = n3->get_current_stat();
		S.es++;
		n3->setup_current_stat(S);
	}
	if( k== 21 )
	    printf("---------------- Start errors ---------------------------\n");
	k++;
    }
    return 0;
}
*/

/* ENGINE FULL TEST  
int main()
{
    unit s,d;
    char type;
    dummy_channel mr1_1,mr1_2,r1r2_1,r1r2_2,r2r3_1,r2r3_2,r3r4_1,r3r4_2,r4r5_1,r4r5_2,r5r6_1,r5r6_2,r6r7_1,r6r7_2,r7s_1,r7s_2;
    EOC_dummy1 *n1 = new EOC_dummy1("m-ns",&mr1_1,&mr1_2);
    
    EOC_dummy1 *n21 = new EOC_dummy1("r1-cs",&mr1_2,&mr1_1);
    EOC_dummy1 *n22 = new EOC_dummy1("r1-ns",&r1r2_1,&r1r2_2);
    
    EOC_dummy1 *n31 = new EOC_dummy1("r2-cs",&r1r2_2,&r1r2_1);    
    EOC_dummy1 *n32 = new EOC_dummy1("r2-ns",&r2r3_1,&r2r3_2);
    
    EOC_dummy1 *n41 = new EOC_dummy1("r3-cs",&r2r3_2,&r2r3_1);    
    EOC_dummy1 *n42 = new EOC_dummy1("r3-ns",&r3r4_1,&r3r4_2);
    
    EOC_dummy1 *n51 = new EOC_dummy1("r4-cs",&r3r4_2,&r3r4_1);    
    EOC_dummy1 *n52 = new EOC_dummy1("r4-ns",&r4r5_1,&r4r5_2);
    
    EOC_dummy1 *n61 = new EOC_dummy1("r5-cs",&r4r5_2,&r4r5_1);    
    EOC_dummy1 *n62 = new EOC_dummy1("r5-ns",&r5r6_1,&r5r6_2);
    
    EOC_dummy1 *n71 = new EOC_dummy1("r6-cs",&r5r6_2,&r5r6_1);    
    EOC_dummy1 *n72 = new EOC_dummy1("r6-ns",&r6r7_1,&r6r7_2);
    
    EOC_dummy1 *n81 = new EOC_dummy1("r7-cs",&r6r7_2,&r6r7_1);    
    EOC_dummy1 *n82 = new EOC_dummy1("r7-ns",&r7s_1,&r7s_2);
    
    
    EOC_dummy1 *n9 = new EOC_dummy1("s-cs",&r7s_2,&r7s_1);
    
    EOC_engine *e1 = new EOC_engine(master,(EOC_dev*)n1,"config");
    EOC_engine *e2 = new EOC_engine(repeater,(EOC_dev*)n22,(EOC_dev*)n21);    
    EOC_engine *e3 = new EOC_engine(repeater,(EOC_dev*)n32,(EOC_dev*)n31);    
    EOC_engine *e4 = new EOC_engine(repeater,(EOC_dev*)n42,(EOC_dev*)n41);    
    EOC_engine *e5 = new EOC_engine(repeater,(EOC_dev*)n52,(EOC_dev*)n51);    
    EOC_engine *e6 = new EOC_engine(repeater,(EOC_dev*)n62,(EOC_dev*)n61);    
    EOC_engine *e7 = new EOC_engine(repeater,(EOC_dev*)n72,(EOC_dev*)n71);    
    EOC_engine *e8 = new EOC_engine(repeater,(EOC_dev*)n82,(EOC_dev*)n81);    

    EOC_engine *e9 = new EOC_engine(slave,(EOC_dev*)n9,"config");    

int k=0;
    while(k<500){
	//sleep(1);
	e1->schedule();
        e2->schedule();
        e3->schedule();
        e4->schedule();
        e5->schedule();
        e6->schedule();
        e7->schedule();
        e8->schedule();
        e9->schedule();
	k++;
	
    }

    k=0;
    printf("--------------------------------------\n");
    while(k<1000){
	e1->schedule();
	k++;
    }
    return 0;
}



*/
/* container test 


class A{
public:
    int a;
    int b;
    int c;
    u32 r;
    u8 s;
};

int main()
{
    EOC_ring_container<shdsl_counters> cont(10);
    cont[0]->tstamp = 1110011;

/*    cont[0]->s = 0;
    for(int i=0;i<10;i++){
	cont.shift(1);
	cont[0]->b = i;
    }
    for(int i=0;i<20;i++){
	if(cont[i]){
	    int k = cont[i]->b;
	    printf("%d-i k = %d\n",i,k);
	}else{
	    printf("%d-i k missing\n",i);
	}	
    }

}

*/

/* DB test 
int main()
{
    sched_queue a;
    unit s,d;
    u8 type;
    sched_elem el;
    a.add(stu_c,BCAST,1,__timestamp(1));
    a.schedule(el,__timestamp(1));
    return 0;

}
*/

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

int main()
{
    dummy_channel ch1,ch2;
    EOC_dummy1 *d1 = new EOC_dummy1(&ch1,&ch2);
    EOC_dummy1 *d2 = new EOC_dummy1(&ch2,&ch1);
    EOC_router *r1 = new EOC_router(master,d1);
    EOC_router *r2 = new EOC_router(slave,d2);
    EOC_msg *m = new EOC_msg(10);
    memcpy(m->payload(),"aaaaaaaa",8);
    m->resize(8);
    m->src(stu_c);
    m->dst(stu_c);
    r1->send(m);
    m = r1->receive();
    return 0;
}
*/
/*
 SCHEDULER TEST

int main()
{
    EOC_scheduler *sch = new EOC_scheduler(2,10);
    unit s,d;
    u8 type;
    sched_elem el;
    EOC_msg *m = new EOC_msg(100);
    m->resize(40);
    sch->link_state(EOC_dev::ONLINE);
    while( !sch->request(el) );
    m->type(el.type+128);
    m->dst(el.src);
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
	while( !sch->request(el) ){
    	    sch->print();	
	    m->type(el.type+128);
	    m->dst(el.src);
	    m->src(el.dst);
	    if( sch->response(m) )
		return -1;
	}
	//sleep(1);
	sch->tick();
    }        
}
*/
