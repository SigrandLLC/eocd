#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>
#include <string.h>

#include <generic/EOC_generic.h>
#include <app-if/app_comm_cli.h>
#include <app-if/app_frame.h>

char *
unit2string(unit u)
{
    static char buf[64];
    switch(u){
    case stu_c:
	sprintf(buf,"STU-C");
	break;
    case stu_r:
	sprintf(buf,"STU-R");
	break;
    case sru1:
	sprintf(buf,"SRU1");
	break;
    case sru2:
	sprintf(buf,"SRU2");
	break;
    case sru3:
	sprintf(buf,"SRU3");
	break;
    case sru4:
	sprintf(buf,"SRU4");
	break;
    case sru5:
	sprintf(buf,"SRU5");
	break;
    case sru6:
	sprintf(buf,"SRU6");
	break;
    case sru7:
	sprintf(buf,"SRU7");
	break;
    case sru8:
	sprintf(buf,"SRU8");
	break;
    default:
	sprintf(buf,"unknown unit");
    }
    return buf;
}

char *
side2string(side s)
{
    static char buf[64];
    switch(s){
    case net_side:
	sprintf(buf,"NetSide");
	break;
    case cust_side:
	sprintf(buf,"CustSide");
	break;
    default:
	sprintf(buf,"unknown side");
	break;
    }
    return buf;
}

int
print_cur_payload(endp_cur_payload *p )
{
    printf("Current state of %s : %s : loop%d\n",unit2string((unit)p->unit),side2string((side)p->side),p->loop);
    printf("LoopAttenuation:\t%d\n",p->cur_attn);
    printf("SNR Margin:\t%d\n",p->cur_snr);
//-------------------------------------------------------------//
    printf("Current Status: ");
    if( p->cur_status.noDefect )
        printf("noDefect ");
    if( p->cur_status.powerBackoff )
        printf("powerBackoff ");
    if( p->cur_status.deviceFault )
        printf("deviceFault ");
    if( p->cur_status.dcContFault )
        printf("dcContFault ");
    if( p->cur_status.snrMargAlarm )
        printf("snrMargAlarm ");
    if( p->cur_status.loopAttnAlarm )
        printf("loopAttnAlarm ");
    if( p->cur_status.loswFailAlarm )
        printf("loswFailAlarm ");
    if( p->cur_status.configInitFailure )
        printf("configInitFailure ");
    if( p->cur_status.protoInitFailure )
        printf("protoInitFailure ");
    if( p->cur_status.noNeighborPresent )
        printf("noNeighborPresent ");
    if( p->cur_status.loopbackActive )
        printf("loopbackActive");
//-------------------------------------------------------------//
    printf("Counters till startup: ");
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u)\n",
	p->total.es,p->total.ses,p->total.crc,p->total.losws,p->total.uas);
//-------------------------------------------------------------//
    printf("Current 15 minutes interval: sec.elapsed(%d) ",p->cur_15m_elaps);
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u)\n",
	p->cur15min.es,p->cur15min.ses,p->cur15min.crc,p->cur15min.losws,p->cur15min.uas);
//-------------------------------------------------------------//
    printf("Current 1 day interval: sec.elapsed(%d) ",p->cur_1d_elaps);
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u)\n",
	p->cur1day.es,p->cur1day.ses,p->cur1day.crc,p->cur1day.losws,p->cur1day.uas);
}

int
print_int_payload(endp_int_payload *p,char *display)
{
    printf("%s interval #%d: ",display,p->int_num);
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u)\n",
	p->cntrs.es,p->cntrs.ses,p->cntrs.crc,p->cntrs.losws,p->cntrs.uas);
    return 0;
}


int
print_endp_cur(app_comm_cli &cli,char *chan,unit u,side s,int loop)
{
    char *b;
    int size;
    // Endpoint current
    endp_cur_payload *p,*p1;
    app_frame *fr, *fr1;

    fr = new app_frame(APP_ENDP_CUR,APP_GET,app_frame::REQUEST,1,chan);
    p = (endp_cur_payload*)fr->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = loop;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
	printf("error requesting\n");
	return -1;
    }
    if( fr1->is_negative() ){ // no such unit or no net_side
	delete fr1;
	printf("Requested component: unit(%s),side(%s),loop(%d) NOT FOUND\n",
		unit2string(u),side2string(s),loop);
    } else {
	p1 = (endp_cur_payload*)fr1->payload_ptr();
	if( p1->unit != p->unit || p1->side != p->side || p1->loop != p->loop ){
	    printf("Error: get information about different unit\n");
	    delete fr1;
	    return -1;
	}
	print_cur_payload(p1);
	delete fr1;
    }
    delete fr;
    return 0;
}


int
print_endp_15m(app_comm_cli &cli,char *chan,unit u,side s,int loop,int inum)
{
    char *b;
    int size;
    // Endpoint current
    endp_15min_payload *p,*p1;
    app_frame *fr, *fr1;

    fr = new app_frame(APP_ENDP_15MIN,APP_GET,app_frame::REQUEST,1,chan);
    p = (endp_15min_payload*)fr->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = loop;
    p->int_num = inum;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
	printf("error requesting\n");
	return -1;
    }
    if( fr1->is_negative() ){ // no such unit or no net_side
	delete fr1;
	printf("Requested component: unit(%s),side(%s),loop(%d),int(%d) NOT FOUND\n",
		unit2string(u),side2string(s),loop,inum);
	delete fr;
	return -1;
    } else {
	p1 = (endp_15min_payload*)fr1->payload_ptr();
	if( p1->unit != p->unit || p1->side != p->side
	    || p1->loop != p->loop || p1->int_num != p->int_num ){
	    printf("Error: get information about different unit\n");
	    delete fr1;
	    return -1;
	}
	print_int_payload(p1,"15min");
	delete fr1;
    }
    delete fr;
    return 0;
}


int
print_endp_1d(app_comm_cli &cli,char *chan,unit u,side s,int loop,int &inum)
{
    char *b;
    int size;
    // Endpoint current
    endp_1day_payload *p,*p1;
    app_frame *fr, *fr1;

    fr = new app_frame(APP_ENDP_1DAY,APP_GET_NEXT,app_frame::REQUEST,1,chan);
    p = (endp_1day_payload*)fr->payload_ptr();
    p->unit = u;
    p->side = s;
    p->loop = loop;
    p->int_num = inum;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
	printf("error requesting\n");
	return -1;
    }
    if( fr1->is_negative() ){ // no such unit or no net_side
	delete fr1;
//	printf("Requested component: unit(%s),side(%s),loop(%d),int(%d) NOT FOUND\n",
//		unit2string(u),side2string(s),loop,inum);
	delete fr;
	return -1;
    } else {
	p1 = (endp_1day_payload*)fr1->payload_ptr();
	if( p1->unit != p->unit || p1->side != p->side
	    || p1->loop != p->loop || p1->int_num != p->int_num ){
	    printf("Error: get information about different unit\n");
	    delete fr1;
	    return -1;
	}
	inum = p1->int_num;
	print_int_payload(p1,"1day");
	printf("MONISECS=%d\n",p1->cntrs.mon_sec);
	delete fr1;
    }
    delete fr;
    return 0;
}

int
main()
{
    char *b,ch,a;
    int i;
    char *iface = "dsl0";
    app_comm_cli cli("/var/eocd/eocd-socket");
    printf("Connect ok\n");

/*
    // profile test
    span_conf_prof_payload *p,*p1;
    app_frame *fr, *fr1;
    char curprof[33];

    curprof[0] = 0;
    while(1){
	fr = new app_frame(APP_SPAN_CPROF,APP_GET_NEXT,app_frame::REQUEST,1,"");
	if( !fr->frame_ptr() ){
	    printf("Error forming request frame\n");
	    return 0;
	}
	p = (span_conf_prof_payload*)fr->payload_ptr();
	strcpy(p->ProfileName,curprof);
	cli.send(fr->frame_ptr(),fr->frame_size());
	cli.wait();
	int size = cli.recv(b);
	fr1 = new app_frame(b,size);
	if( !fr1->frame_ptr() ){
	    printf("error requesting\n");
	    return -1;
	}else if(fr1->is_negative() ){
	    printf("-------- End -------------\n");
	    break;
	}
	p1 = (span_conf_prof_payload*)fr1->payload_ptr();
	printf("%s: annex=%d,min_rate=%d,mxrate=%d,lprobe=%d,wires=%d\n",
		p1->ProfileName,p1->conf.annex,p1->conf.min_rate,
		p1->conf.max_rate,p1->conf.line_probe,
		p1->conf.wires);
	curprof[0] = 0;
	strcpy(curprof,p1->ProfileName);
    }


/*
printf("-----------------------------------------------\n");
    print_endp_cur(cli,iface,stu_c,net_side,0);
printf("-----------------------------------------------\n");
    print_endp_cur(cli,iface,stu_r,cust_side,0);
printf("-----------------------------------------------\n");
    print_endp_cur(cli,iface,sru1,net_side,0);
printf("-----------------------------------------------\n");
    print_endp_cur(cli,iface,sru1,cust_side,0);
printf("-----------------------------------------------\n");
*/
    i =0;
    while( !print_endp_1d(cli,"dsl2",stu_c,cust_side,0,i) ){
	i++;
    }
    i =0;
    while( !print_endp_1d(cli,"dsl2",stu_r,net_side,0,i) ){
	i++;
    }


/*
printf("-----------------------------------------------\n");
    i =0;
    while( !print_endp_15m(cli,"dsl2",stu_r,cust_side,0,i) ){
	i++;
    }
printf("-----------------------------------------------\n");
    i =0;
    while( !print_endp_15m(cli,"dsl2",sru1,cust_side,0,i) ){
	i++;
    }
printf("-----------------------------------------------\n");
    i =0;
    while( !print_endp_15m(cli,"dsl2",sru1,net_side,0,i) ){
	i++;
    }
printf("-----------------------------------------------\n");
    i =0;
    while( !print_endp_15m(cli,"dsl2",sru2,cust_side,0,i) ){
	i++;
    }
printf("-----------------------------------------------\n");
    i =0;
    while( !print_endp_15m(cli,"dsl2",sru2,net_side,0,i) ){
	i++;
    }
printf("-----------------------------------------------\n");
*/
    return 0;
}


/*
    app_frame *fr = new app_frame(app_frame::SPAN_STATUS,app_frame::GET,app_frame::REQUEST,"dsl0");
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    printf("Receiving 1-st frame\n");
    int size = cli.recv(b);
    printf("Received %d bytes\n",size);
    app_frame *fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() ){
	printf("Error in frame\n");
	return -1;
    }
    span_status_payload *p = (span_status_payload *)fr1->payload_ptr();
    printf("Act reps = %d\n",p->nreps);

    fr = new app_frame(app_frame::SPAN_CONF,app_frame::GET,app_frame::REQUEST,"dsl0");
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    span_conf_payload *p1 = (span_conf_payload *)fr1->payload_ptr();
    printf("Conf reps = %d\n",p1->nreps);

    printf("Request STU-C inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    inventory_payload *p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = stu_c;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

    printf("Request STU-R inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = stu_r;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

    printf("Request SRU1 inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = sru1;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

    printf("Request SRU2 inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = sru2;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

    // Endpoint current
    printf("Request STU-C current info\n");
    endp_cur_payload *p3;
    fr = new app_frame(app_frame::ENDP_CUR,app_frame::GET,app_frame::REQUEST,"dsl0");
    p3 = (endp_cur_payload*)fr->payload_ptr();
    p3->unit = sru2;
    p3->side = EOC_unit::net_side;
    p3->loop = 1;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
    }

    printf("Request STU-R current info\n");
    fr = new app_frame(app_frame::ENDP_CUR,app_frame::GET,app_frame::REQUEST,"dsl0");
    p3 = (endp_cur_payload*)fr->payload_ptr();
    p3->unit = sru2;
    p3->side = EOC_unit::net_side;
    p3->loop = 1;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
    }


    ENDP_15MIN

    printf("Request SRU2 inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = sru2;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();
	printf("Manuf = %s\n",p2->inv.ven_id);
    }


    ENDP_1DAY
    printf("Request SRU2 inventory\n");
    fr = new app_frame(app_frame::INVENTORY,app_frame::GET,app_frame::REQUEST,"dsl0");
    p2 = (inventory_payload*)fr->payload_ptr();
    p2->unit = sru2;
    cli.send(fr->frame_ptr(),fr->frame_size());
    cli.wait();
    size = cli.recv(b);
    fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	printf("error requesting\n");
    } else {
	p2 = (inventory_payload *)fr1->payload_ptr();
	printf("Manuf = %s\n",p2->inv.ven_id);
    }

*/
