/* TEST program */


extern "C" {
#include<malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
}

#include <devs/EOC_dummy1.h>
#include <db/EOC_loop.h>
#include <utils/EOC_ring_container.h>
#include <db/EOC_db.h>
#include <engine/EOC_engine.h>
#include <engine/EOC_engine_act.h>

#include <EOC_main.h>
#include <eoc_debug.h>

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


EOC_dummy1 *Nx[15][2];

EOC_dev_terminal *
init_dev(char *name_1)
{

    if( !strcmp(name_1,"dsl0") ){
	return Nx[0][0];
    }
    if( !strcmp(name_1,"dsl1") ){
	return Nx[1][0];
    }
    if( !strcmp(name_1,"dsl2") ){
	return Nx[2][0];
    }
    if( !strcmp(name_1,"dsl3") ){
	return Nx[3][0];
    }
    if( !strcmp(name_1,"dsl4") ){
	return Nx[4][0];
    }
    if( !strcmp(name_1,"dsl5") ){
	return Nx[5][0];
    }
    if( !strcmp(name_1,"dsl6") ){
	return Nx[6][0];
    }
    if( !strcmp(name_1,"dsl7") ){
	return Nx[7][0];
    }
    if( !strcmp(name_1,"dsl8") ){
	return Nx[8][0];
    }
    if( !strcmp(name_1,"dsl9") ){
	return Nx[9][0];
    }
    if( !strcmp(name_1,"dsl10") ){
	return Nx[10][0];
    }
    if( !strcmp(name_1,"dsl11") ){
	return Nx[11][0];
    }
    if( !strcmp(name_1,"dsl12") ){
	return Nx[12][0];
    }
    if( !strcmp(name_1,"dsl13") ){
	return Nx[13][0];
    }
    if( !strcmp(name_1,"dsl14") ){
	return Nx[14][0];
    }

    return NULL;
}


/*

int main()
{
    unit s,d;
    char type;

    printf("Hi!!!\n");

    dummy_channel mr1_1,mr1_2,r1s_1,r1s_2;
    dummy_channel tmp1_1,tmp1_2;
    dummy_channel tmp2_1,tmp2_2;
    dummy_channel tmp3_1,tmp3_2;
    dummy_channel tmp4_1,tmp4_2;
    dummy_channel tmp5_1,tmp5_2;
    dummy_channel tmp6_1,tmp6_2;
    dummy_channel tmp7_1,tmp7_2;
    dummy_channel tmp8_1,tmp8_2;



    n1 = new EOC_dummy1("m-ns",&mr1_1,&mr1_2);
    n21 = new EOC_dummy1("r1-cs",&mr1_2,&mr1_1);
    n22 = new EOC_dummy1("r1-ns",&r1s_1,&r1s_2);
    n3 = new EOC_dummy1("s-cs",&r1s_2,&r1s_1);

    n4 = new EOC_dummy1("a1",&tmp1_1,&tmp1_2);
    n5 = new EOC_dummy1("a1",&tmp2_1,&tmp2_2);
    n6 = new EOC_dummy1("a1",&tmp3_1,&tmp3_2);
    n7 = new EOC_dummy1("a1",&tmp4_1,&tmp4_2);
    n8 = new EOC_dummy1("a1",&tmp5_1,&tmp5_2);
    n9 = new EOC_dummy1("a1",&tmp6_1,&tmp6_2);
    n10 = new EOC_dummy1("a1",&tmp7_1,&tmp7_2);
    n11 = new EOC_dummy1("a1",&tmp8_1,&tmp8_2);



    EOC_main m("eocd.cfg");
    EOC_engine *e2 = new EOC_engine(n22,n21);

    printf("All successfull,starting\n");

    int k=0;
    side_perf S;
    while(k<200){
//	sleep(1);
	k++;
//	m.app_listen();
	m.poll_channels();
	e2->schedule();
	if( k>40 ){
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
    }

    while(1){
	m.poll_channels();
	e2->schedule();
	m.app_listen(2);

	if( k &>40 ){
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

    }
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

    EOC_engine *e1 = new EOC_engine_act(n1,3);
    EOC_engine *e2 = new EOC_engine(n22,n21);
    EOC_engine *e3 = new EOC_engine(n3);

int k=0;
    side_perf S;
    while(k<500){
//	sleep(1);
	e1->schedule();
        e2->schedule();
        e3->schedule();
	if( k>40 ){
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

/* ENGINE FULL TEST  */
int main()
{
    unit s,d;
    char type;

// ------------------ Channel2 ------------------------------------------------------
#define REPEATERS 2
    dummy_channel CHANS[15][REPEATERS+1][2];
    EOC_dummy1 *Nxx[15][REPEATERS][2];
    EOC_engine *Ex[15][REPEATERS];
    EOC_engine *E[15];

    for(int i=0;i<15;i++){
	Nx[i][0] = new EOC_dummy1("m-cs",&CHANS[i][0][0],&CHANS[i][0][1]);
	Nx[i][1] = new EOC_dummy1("s-ns",&CHANS[i][REPEATERS][1],&CHANS[i][REPEATERS][0]);
	E[i] = new EOC_engine(Nx[i][1],slave);
        for(int kk=0;kk<REPEATERS;kk++){
	    Nxx[i][kk][0] = new EOC_dummy1("R",&CHANS[i][kk][1],&CHANS[i][kk][0]);
	    Nxx[i][kk][1] = new EOC_dummy1("R",&CHANS[i][kk+1][0],&CHANS[i][kk+1][1]);
	    Ex[i][kk] = new EOC_engine(Nxx[i][kk][0],Nxx[i][kk][1]);
	}
    }



    EOC_main m("eocd.conf","/home/artpol/");

//----------------------------------------------------------------

    int k=0;
    side_perf S;

    debug_lev = DINFO;
    k = 0;
    while(1){
	m.poll_channels();
	m.app_listen(2);
	for(int i=0;i<1;i++){
	    E[i]->schedule();
	    for(int kk=0;kk<1;kk++)
	        Ex[i][kk]->schedule();
	}
	k++;
/*
	if( !(k%40) ){
		S = Nxx[0][1]->get_current_stat();
		S.ses++;
		Nxx[0][1]->setup_current_stat(S);


		S = Nxx[0][0]->get_current_stat();
		S.es++;
		S.losws++;
		Nxx[0][0]->setup_current_stat(S);

		S = Nxx[1][0]->get_current_stat();
		S.crc++;
		Nxx[1][0]->setup_current_stat(S);

		S = Nxx[1][1]->get_current_stat();
		S.uas++;
		Nxx[1][1]->setup_current_stat(S);
	}
*/
    }
    return 0;
}




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
