#include <stdio.h>
#include <string.h>

#include "comm.h"

int 
print_int_payload(endp_int_payload *p,char *display)
{
    printf("%s interval #%d: ",display,p->int_num);	
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u)\n",
	p->cntrs.es,p->cntrs.ses,p->cntrs.crc,p->cntrs.losws,p->cntrs.uas);
    return 0;
}

int main()
{
    struct app_comm *c = init_comm();
    struct app_frame *fr1,*fr2;
    struct app_frame *fr3,*fr4;
    char *b,*b1;
    span_name_payload *p;
    span_params_payload *p1;

    int i;
    
    if(!c){
	printf("Cannot initialise connection to eocd\n");
	return -1;
    }

    char if_name[SPAN_NAME_LEN];
    if_name[0] = 0;
    
    while(1){
        p = (span_name_payload*)comm_alloc_request(APP_SPAN_NAME,APP_GET,if_name,&fr1);
	if( !p ){
	    printf("Cannot allocate application frame\n");
	    return -1;
	}
	fr2 = comm_request(c,fr1);
	if( !fr2 ){
	    printf("Error requesting\n");
	    return -1;
	}
	p = (span_name_payload*)comm_frame_payload(fr2);
	printf("Channels: %d\n",p->filled);
	for(i=0;i<p->filled;i++){
	    printf("ch%d: %s ",i,p->name[i]);
	    p1 = (span_params_payload*)comm_alloc_request(APP_SPAN_PARAMS,APP_GET,p->name[i],&fr3);
	    if( !p1 ){
		printf("Cannot allocate application frame\n");
		return -1;
	    }
	    fr4 = comm_request(c,fr3);
	    if( !fr4 ){
		printf(" params unknown\n");
		continue;
	    }
	    p1 = (span_params_payload*)comm_frame_payload(fr4);
	    printf("units(%d) loops(%d)\n",p1->units,p1->loops);
	}
	
	if( !p->last_msg && p->filled){
	    strncpy(if_name,p->name[p->filled-1],SPAN_NAME_LEN);
	} else {
	    break;
	}
    }
}