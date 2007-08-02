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


int
main()
{
    char *b,ch,a;
    int i;
    app_comm_cli cli("socket");
    printf("Connect ok\n");
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


    
    return 0;
}
