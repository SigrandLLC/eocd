#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "../comm.h"
/*
int 
print_int_payload(endp_int_payload *p,char *display)
{
    printf("%s interval #%d: ",display,p->int_num);	
    printf("es(%u) ses(%u) crc(%d) losws(%u) uas(%u)\n",
	p->cntrs.es,p->cntrs.ses,p->cntrs.crc,p->cntrs.losws,p->cntrs.uas);
    return 0;
}


int
Interface_Index_By_Name(char *name_1,int s)
{
    if( !strcmp(name_1,"eth0") ){
	return 1;
    }	
    if( !strcmp(name_1,"dsl1") ){
	return 2;
    }

    if( !strcmp(name_1,"dsl2") ){
	return 3;
    }	
    if( !strcmp(name_1,"dsl3") ){
	return 4;
    }
    if( !strcmp(name_1,"dsl4") ){
	return 5;
    }	
    if( !strcmp(name_1,"dsl5") ){
	return 6;
    }
    if( !strcmp(name_1,"dsl6") ){
	return 7;
    }	
    if( !strcmp(name_1,"dsl7") ){
	return 8;
    }
    if( !strcmp(name_1,"dsl8") ){
	return 9;
    }	
    if( !strcmp(name_1,"dsl9") ){
	return 10;
    }

    return -1;
}





struct app_comm *comm;

#define SHDSL_MAX_CHANNELS 30
typedef struct{
    char name[SPAN_NAME_LEN];
    int index;
} shdsl_channel_elem;


int
chann_names(shdsl_channel_elem *tbl,int *min_i)
{
    struct app_frame *fr1,*fr2;
    span_name_payload *p;
    int i;
    int tbl_size = 0;
    char ifname[SPAN_NAME_LEN];
    ifname[0] = 0;
    
    do{    
	p = (span_name_payload*)
		comm_alloc_request(APP_SPAN_NAME,APP_GET,ifname,&fr1);

	if( !p ){
//	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	    printf("Cannot allocate application frame\n");
	    return -1;
	}

	fr2 = comm_request(comm,fr1);
	if( !fr2 ){
//	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Reqest failed"));
	    printf("Reqest failed");
	    return -1;
	}

	p = (span_name_payload*)comm_frame_payload(fr2);
	for(i=0;i<p->filled;i++){
	    strncpy(tbl[tbl_size].name,p->name[i],SPAN_NAME_LEN);
	    tbl[tbl_size].index = Interface_Index_By_Name(p->name[i],
					strnlen(p->name[i],SPAN_NAME_LEN));
	    if( (i==0) || tbl[*min_i].index > tbl[i].index  ){
		*min_i = i;
	    }
	    tbl_size++;
	}
	if( !p->last_msg && p->filled){
	    strncpy(ifname,p->name[p->filled-1],SPAN_NAME_LEN);
	}
    }while( tbl_size<SHDSL_MAX_CHANNELS && !p->last_msg );
    return tbl_size;
}
*/

struct app_comm *comm;

#define CACHE_INT 1
#define SHDSL_MAX_CHANNELS 30
typedef struct{
    struct timeval tv;
    char name[SPAN_NAME_LEN];
    int index;
    int units;
    int wires;
} shdsl_channel_elem;

typedef struct{
    struct timeval tv;
    span_conf_payload p;
} shdsl_spanconf_elem;

typedef struct{
    struct timeval tv;
    span_status_payload p;
} shdsl_spanstatus_elem;

// -------- Served channel names cache -------------//
shdsl_channel_elem tbl[SHDSL_MAX_CHANNELS];
int tbl_size;
struct timeval tbl_tv = {0,0};
int min_i = 0;

char interface_ind;

int
ifname_to_index(char *Name, int Len)
{
    short ifIndex = 0;
    char ifName[20];
    char found = 0;
    if( !strcmp(Name,"dsl2") )
	return 10;
    return -1;
}
/*
int
Interface_Index_By_Name(char *name_1,int s)
{
    if( !strcmp(name_1,"eth0") ){
	return 1;
    }	
    if( !strcmp(name_1,"dsl1") ){
	return 2;
    }

    if( !strcmp(name_1,"dsl2") ){
	return 3;
    }	
    if( !strcmp(name_1,"dsl3") ){
	return 4;
    }
    if( !strcmp(name_1,"dsl4") ){
	return 5;
    }	
    if( !strcmp(name_1,"dsl5") ){
	return 6;
    }
    if( !strcmp(name_1,"dsl6") ){
	return 7;
    }	
    if( !strcmp(name_1,"dsl7") ){
	return 8;
    }
    if( !strcmp(name_1,"dsl8") ){
	return 9;
    }	
    if( !strcmp(name_1,"dsl9") ){
	return 10;
    }

    return -1;
}
*/


int
chann_names()
{
    struct app_frame *fr1,*fr2;
    span_name_payload *p;
    int i=0 ;
    char ifname[SPAN_NAME_LEN];
    int index,len;
    // caching
    struct timeval tvcur;
    char tverr = 0;

    if( gettimeofday(&tvcur,NULL) )
	tverr = 1;
	
//    if( ((tvcur.tv_sec - tbl_tv.tv_sec) > CACHE_INT) || tverr ){

        ifname[0] = 0;
	tbl_size = 0;

	p = (span_name_payload*)
	    comm_alloc_request(APP_SPAN_NAME,APP_GET,ifname,&fr1);

	if( !p ){
	
    	    return -1;
	}
    
        do{
	    set_chan_name(fr1,ifname);
	    fr2 = comm_request(comm,fr1);
	    if( !fr2 ){
		printf("mibII/hdsl2Shdsl Reqest failed\n");
		comm_frame_free(fr1);
		return -1;
	    }
	    p = (span_name_payload*)comm_frame_payload(fr2);
	    for(i=0;i<p->filled;i++){
		len = strnlen(p->name[i],SPAN_NAME_LEN);
		if( (index = ifname_to_index(p->name[i],len)) < 0 )
		    continue;
		tbl[tbl_size].name[0];
		strncpy(tbl[tbl_size].name,p->name[i],len);
//	    	printf("Get sys insex for dev %s: %d\n",p->name[i],index);
		tbl[tbl_size].index = index;
		if( (tbl_size==0) || tbl[min_i].index > tbl[i].index  ){
		    min_i = i;
		}
		tbl[tbl_size].units = -1;
		tbl[tbl_size].wires = -1;
		tbl[tbl_size].tv.tv_sec = 0;
		tbl_size++;
	    }

	    if( !p->last_msg && p->filled){
		strncpy(ifname,p->name[p->filled-1],SPAN_NAME_LEN);
	    }
	    comm_frame_free(fr2);
	}while( tbl_size<SHDSL_MAX_CHANNELS && !p->last_msg );
	comm_frame_free(fr1);
	tbl_tv = tvcur;
//    }
    return min_i;
}



int main()
{
    int i = 0,j;
    while(1){
	i++;
        if( !(comm = init_comm(1)) ){
	    printf("Error while connecting\n");
	    return 0;
	}
	chann_names();
	printf("Iter %d:\n",i);
	for(j=0;j<tbl_size;j++){
	    printf("%s ",tbl[j].name);
	}
	printf("\n");
	comm_free(comm);
    }
    return 0;
}



//inventory_info(char 

/*
int main()
{
    comm = init_comm();
    shdsl_channel_elem tbl[SHDSL_MAX_CHANNELS];
    int tbl_size,min_i;
    int i;
    struct app_frame *fr1,*fr2;
    endp_int_payload *p;
//    tbl_size = chann_names(tbl,&min_i);

    for(i=1;1;i++){
        p = (endp_int_payload*)comm_alloc_request(APP_ENDP_15MIN,APP_GET,"dsl2",&fr1);
	if( !p ){
//	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
    	    printf("Cannot allocate application frame\n");
    	    return -1;
	}

	p->unit = stu_r;
	p->side = cust_side;
	p->loop = 0;
	p->int_num = i;

        fr2 = comm_request(comm,fr1);
	if( !fr2 ){
//	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Reqest failed"));
    	    printf("Reqest failed");
    	    return -1;
	}
	
        p = (endp_int_payload*)comm_frame_payload(fr2);
	printf("int#%d\n",i);
    }
}




/*
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


*/
