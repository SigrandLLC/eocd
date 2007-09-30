#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include <net-snmp/net-snmp-config.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/agent/auto_nlist.h>

#include <stdio.h>
#include <time.h>

#include "shdsl.h"
#include "struct.h"
#include "util_funcs.h"
#include "../sysORTable.h"
#include "../interfaces.h"

#include <app-if/app_messages.h>

//------- Global definitions -------------//
#define CACHE_INT 5
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

// -------- Served channels span info -------------//
shdsl_spanconf_elem *spanconf_tbl[SHDSL_MAX_CHANNELS];
int spanconf_tbl_size;
shdsl_spanstatus_elem *spanstatus_tbl[SHDSL_MAX_CHANNELS];
int spanstatus_tbl_size;


/*---- global vars ----*/
struct app_comm *comm;
char driver_dir_path[]="/root/snmp/";


char interface_ind;
int unit_index = 0;
int endp_index = 0;
int wire_index = 0;
counters_t perf_int;

//------- DEBUG ------------//
void
dbg_oid(char *comment,char *name, int length)
{
    DEBUGMSGTL(("mibII/shdsl", comment));
    DEBUGMSGOID(("mibII/shdsl", name, length));
    DEBUGMSG(("mibII/shdsl", "\n"));    
}


/*------------------------------ mib registration data -----------------------------------*/

oid hdsl2Shdsl_variables_oid[] = { SNMP_OID_MIB2, 10, 48, 1 };
struct variable3 shdsl_spanconf[] = {
    { CONF_NREPS, ASN_GAUGE,RWRITE, var_SpanConfEntry, 3, {1,1,1} },
    { CONF_PRFL, ASN_OCTET_STR, RWRITE, var_SpanConfEntry, 3, {1,1,2} },    
    { CONF_ALARMPRFL, ASN_OCTET_STR, RWRITE, var_SpanConfEntry, 3, {1,1,3} },    
};

struct variable3 shdsl_spanstat[] = {
    { STATUS_NAVAILREPS, ASN_UNSIGNED, RONLY, var_SpanStatusEntry, 3, {2,1,1} },
    { STATUS_MAXATTLRATE, ASN_UNSIGNED, RONLY, var_SpanStatusEntry, 3, {2,1,2} },    
    { STATUS_ACTLRATE, ASN_UNSIGNED, RONLY, var_SpanStatusEntry, 3, {2,1,3} },    
    { STATUS_TRNSMSNMODCUR, ASN_BIT_STR, RONLY, var_SpanStatusEntry, 3, {2,1,4} },
    { STATUS_MAXATTPRATE, ASN_UNSIGNED, RONLY, var_SpanStatusEntry, 3, {2,1,5} },    
    { STATUS_ACTPRATE, ASN_UNSIGNED, RONLY, var_SpanStatusEntry, 3, {2,1,6} },        
};

struct variable3 shdsl_inventory[] = {
    {INV_INDEX, ASN_INTEGER, NOACCESS, var_InventoryEntry, 3, {3,1,1}},
    {INV_VID, ASN_OCTET_STR, RONLY, var_InventoryEntry, 3, {3,1,2}},    
    {INV_VMODELNUM, ASN_OCTET_STR, RONLY, var_InventoryEntry, 3, {3,1,3}},    
    {INV_VSERNUM, ASN_OCTET_STR, RONLY, var_InventoryEntry, 3, {3,1,4}},
    {INV_VEOCSV, ASN_INTEGER, RONLY, var_InventoryEntry, 3, {3,1,5}},    
    {INV_STANDARDV, ASN_INTEGER, RONLY, var_InventoryEntry, 3, {3,1,6}},    
    {INV_VLISTNUM, ASN_OCTET_STR, RONLY, var_InventoryEntry, 3, {3,1,7}},        
    {INV_VISSUENUM, ASN_OCTET_STR, RONLY, var_InventoryEntry, 3, {3,1,8}},    
    {INV_VSOFTWV,ASN_OCTET_STR , RONLY, var_InventoryEntry, 3, {3,1,9}},
    {INV_EQCODE, ASN_OCTET_STR, RONLY, var_InventoryEntry, 3, {3,1,10}},    
    {INV_VOTHER, ASN_OCTET_STR, RONLY, var_InventoryEntry, 3, {3,1,11}},
    {INV_TRNSMODECPB, ASN_BIT_STR, RONLY, var_InventoryEntry, 3, {3,1,12}}    
};

/*
struct variable3 shdsl_endp_conf[] = {
    {ENDP_CONF_PROF, ASN_OCTET_STR, RONLY, var_EndpointConfEntry, 3, {4,1,3}},    
};
*/

struct variable3 shdsl_endp_currstat[] = {
    { ENDP_STAT_CUR_ATN, ASN_INTEGER, RONLY, var_EndpointCurrEntry, 3, {5,1,1} },
    { ENDP_STAT_CUR_SNRMGN, ASN_INTEGER, RONLY, var_EndpointCurrEntry, 3, {5,1,2} },
    { ENDP_STAT_CUR_STATUS,ASN_BIT_STR,RONLY, var_EndpointCurrEntry, 3, {5,1,3} },
    { ENDP_STAT_CUR_ES,ASN_COUNTER,RONLY, var_EndpointCurrEntry, 3, {5,1,4} },
    { ENDP_STAT_CUR_SES,ASN_COUNTER,RONLY, var_EndpointCurrEntry, 3, {5,1,5} },
    { ENDP_STAT_CUR_CRC,ASN_COUNTER,RONLY, var_EndpointCurrEntry, 3, {5,1,6} },
    { ENDP_STAT_CUR_LOSWS,ASN_COUNTER,RONLY, var_EndpointCurrEntry, 3, {5,1,7} },
    { ENDP_STAT_CUR_UAS,ASN_COUNTER,RONLY, var_EndpointCurrEntry, 3, {5,1,8} },
    { ENDP_STAT_CUR_15MEL,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,9} },
    { ENDP_STAT_CUR_15M_ES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,10} },
    { ENDP_STAT_CUR_15M_SES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,11} },
    { ENDP_STAT_CUR_15M_CRC,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,12} },
    { ENDP_STAT_CUR_15M_LOSWS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,13} },
    { ENDP_STAT_CUR_15M_UAS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,14} },
    { ENDP_STAT_CUR_1DEL,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,15} },
    { ENDP_STAT_CUR_1D_ES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,16} },
    { ENDP_STAT_CUR_1D_SES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,17} },
    { ENDP_STAT_CUR_1D_CRC,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,18} },
    { ENDP_STAT_CUR_1D_LOSWS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,19} },
    { ENDP_STAT_CUR_1D_UAS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,20} },
};

struct variable3 shdsl_endp_15minstat[] = {
    { ENDP_15M_INT,ASN_UNSIGNED,NOACCESS,var_15MinIntervalEntry,3,{6,1,1} },
    { ENDP_15M_ES,ASN_UNSIGNED,RONLY,var_15MinIntervalEntry,3,{6,1,2} },
    { ENDP_15M_SES,ASN_UNSIGNED,RONLY,var_15MinIntervalEntry,3,{6,1,3} },
    { ENDP_15M_CRC,ASN_UNSIGNED,RONLY,var_15MinIntervalEntry,3,{6,1,4} },
    { ENDP_15M_LOSWS,ASN_UNSIGNED,RONLY,var_15MinIntervalEntry,3,{6,1,5} },
    { ENDP_15M_UAS,ASN_UNSIGNED,RONLY,var_15MinIntervalEntry,3,{6,1,6} },
};
/*
struct variable3 shdsl_endp_1daystat[] = {
    { ENDP_1D_ES,ASN_UNSIGNED,NOACCESS,var_1DayIntervalEntry,3,{6,1,1} },
    { ENDP_1D_SES,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{6,1,2} },
    { ENDP_1D_CRC,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{6,1,3} },
    { ENDP_1D_LOSWS,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{6,1,4} },
    { ENDP_1D_UAS,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{6,1,5} },
};
*/
/*
struct variable3 shdsl_endp_maint[] = {
    { ENDP_MAINT_LOOPBACK, ASN_INTEGER, RWRITE, var_EndpointMaintEntry, 3, {8,1,1} },
    { ENDP_MAINT_TIPRINGREV, ASN_INTEGER, RONLY, var_EndpointMaintEntry, 3, {8,1,2} },
    { ENDP_MAINT_PWRBACKOFF, ASN_INTEGER, RWRITE, var_EndpointMaintEntry, 3, {8,1,3} },
    { ENDP_MAINT_SOFTRESTART, ASN_INTEGER, RWRITE, var_EndpointMaintEntry, 3, {8,1,4} },
};

struct variable3 shdsl_unit_maint[] = {
    {UNIT_MAINT_LPB_TO, ASN_INTEGER, RWRITE, var_UnitMaintEntry, 3, {9,1,1}},
    {UNIT_MAINT_PWR_SRC, ASN_INTEGER, RONLY, var_UnitMaintEntry, 3, {9,1,2}},
};

struct variable3 shdsl_conf_prof[] = {
    {CONF_WIRE_IFACE, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,2}},
    {CONF_MIN_LRATE, ASN_UNSIGNED, RWRITE, var_SpanConfProfEntry, 3, {10,1,3}},
    {CONF_MAX_LRATE, ASN_UNSIGNED, RWRITE, var_SpanConfProfEntry, 3, {10,1,4}},
    {CONF_PSD, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,5}},
    {CONF_TRNSM_MODE, ASN_BIT_STR, RWRITE, var_SpanConfProfEntry, 3, {10,1,6}},
    {CONF_REM_ENABLE, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,7}},
    {CONF_PWR_FEED, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,8}},
    {CONF_CURR_DOWN, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,9}},
    {CONF_WORST_DOWN, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,10}},
    {CONF_CURR_UP, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,11}},
    {CONF_WORST_UP, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,12}},
    {CONF_USED_MARG, ASN_BIT_STR, RWRITE, var_SpanConfProfEntry, 3, {10,1,13}},
    {CONF_REF_CLK, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,14}},
    {CONF_LPROBE, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,15}},
    {CONF_ROW_ST, ASN_INTEGER, RWRITE, var_SpanConfProfEntry, 3, {10,1,16}},
};
*/



void
init_shdsl(void)
{

    comm = NULL;
    memset(&perf_int,0,sizeof(perf_int));
    memset(spanconf_tbl,0,sizeof(spanconf_tbl));
    spanconf_tbl_size = 0;
    memset(spanstatus_tbl,0,sizeof(spanstatus_tbl));
    spanstatus_tbl_size = 0;
    memset(tbl,0,sizeof(tbl));

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/hdsl2ShdslSpanConf", shdsl_spanconf, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslSpanStatus", shdsl_spanstat, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslInventory", shdsl_inventory, variable3,
                 hdsl2Shdsl_variables_oid);

/*
    REGISTER_MIB("mibII/hdsl2shdslEndpointConf", shdsl_endp_conf, variable3,
                 hdsl2Shdsl_variables_oid);
*/
    REGISTER_MIB("mibII/hdsl2shdslEndpointCurr",shdsl_endp_currstat, variable3,
                 hdsl2Shdsl_variables_oid);

/*
    REGISTER_MIB("mibII/hdsl2shdslEndpoint15min",shdsl_endp_15minstat, variable3,
                 hdsl2Shdsl_variables_oid);
/*
    REGISTER_MIB("mibII/hdsl2shdslEndpoint1day",shdsl_endp_1daystat, variable3,
                 hdsl2Shdsl_variables_oid);

/*
    REGISTER_MIB("mibII/hdsl2shdslEndpointMaint", shdsl_endp_maint, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslUnitMaint", shdsl_unit_maint, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslSpanConf", shdsl_conf_prof, variable3,
                 hdsl2Shdsl_variables_oid);

*/

    DEBUGMSGTL(("mibII/hdsl2Shdsl","register variables"));
}

/*
 * header_dslIfIndex(...
 * Arguments:
 * vp     IN      - pointer to variable entry that points here
 * name    IN/OUT  - IN/name requested, OUT/name found
 * length  IN/OUT  - length of IN/OUT oid's 
 * exact   IN      - TRUE if an exact match was requested
 * var_len OUT     - length of variable or 0 if function returned
 * write_method
 */

int
ifname_to_index(char *Name, int Len)
{
    short ifIndex = 0;
    char ifName[20];
    char found = 0;
    Interface_Scan_Init();
    while ( Interface_Scan_Next(&ifIndex, ifName, NULL, NULL) ){
	if( !strcmp(Name, ifName)){
	    found = 1;
	    break;
	}
    }
    if( found )
	return ifIndex;
    
    return -1;
}

/*
int
ifname_to_index(char *name_1, int Len)
{
    if( !strcmp(name_1,"eth0") ){
	return 2;
    }	
    if( !strcmp(name_1,"dsl1") ){
	return 3;
    }

    if( !strcmp(name_1,"dsl2") ){
	return 4;
    }	
    if( !strcmp(name_1,"dsl3") ){
	return 5;
    }
    if( !strcmp(name_1,"dsl4") ){
	return 6;
    }	
    if( !strcmp(name_1,"dsl5") ){
	return 7;
    }
    if( !strcmp(name_1,"dsl6") ){
	return 8;
    }	
    if( !strcmp(name_1,"dsl7") ){
	return 9;
    }
    if( !strcmp(name_1,"dsl8") ){
	return 10;
    }	
    if( !strcmp(name_1,"dsl9") ){
	return 11;
    }

    return -1;
}
*/



int
chann_names()
{
    struct app_frame *fr1,*fr2;
    span_name_payload *p;
    int i;
    char ifname[SPAN_NAME_LEN];
    int index,len;
    // caching
    struct timeval tvcur;
    char tverr = 0;

    if( gettimeofday(&tvcur,NULL) )
	tverr = 1;
	
    if( ((tvcur.tv_sec - tbl_tv.tv_sec) > CACHE_INT) || tverr ){

        ifname[0] = 0;
	tbl_size = 0;
	p = (span_name_payload*)
	    comm_alloc_request(APP_SPAN_NAME,APP_GET,ifname,&fr1);

	if( !p ){
    	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
    	    comm_frame_free(fr1);
    	    return -1;
	}
    
        do{
	    set_chan_name(fr1,ifname);
	    fr2 = comm_request(comm,fr1);
	    if( !fr2 ){
//	    	DEBUGMSGTL(("mibII/hdsl2Shdsl","Reqest failed"));
		printf("mibII/hdsl2Shdsl Reqest failed\n");
		comm_frame_free(fr1);
		return -1;
	    }
	    p = (span_name_payload*)comm_frame_payload(fr2);
	    for(i=0;i<p->filled;i++){
		len = strnlen(p->name[i],SPAN_NAME_LEN);
		if( (index = ifname_to_index(p->name[i],len)) < 0 )
		    continue;
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
    }

    return min_i;
}


static int
header_ifIndex(struct variable *vp,
               oid * name, size_t * length, int exact,
	       size_t * var_len, WriteMethod ** write_method)
{
    oid newname[MAX_OID_LEN];
    int base_compare;
    int oid_min = (vp->namelen > *length) ? *length : vp->namelen;
    int min_ind = -1, min_i = 0;
    int i;

//    printf("ifIndex: DSL indexes:\n");    
    if( (min_i = chann_names()) < 0 ){
	printf("ifIndex: Cannot get table of controlling interfaces\n");
	return -1;
    }

//    printf("ifIndex: name[%d]=%d\n",i,name[i]);


    if( !tbl_size ){
	printf("ifIndex: MATCH FAILED: tbl_size is zero\n");
	return MATCH_FAILED;
    }

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));

    if( (base_compare = snmp_oid_compare(name,oid_min,newname,oid_min)) > 0){
//	DEBUGMSGTL(("mibII/shdsl", "GETNEXT with greater OID than base OID\n"));  
//	printf("ifIndex: GETNEXT with greater OID than base OID\n");  
	return MATCH_FAILED;
    }
    
    if( exact ){
	if( base_compare || (*length < vp->namelen+1) ){
//	    printf("ifIndex: base_compare || (*length < vp->namelen+1)\n");
	    return MATCH_FAILED;
	}
	// check that name[vp->namelen] is DSL index
	for(i=0;i<tbl_size;i++){
	    if( name[vp->namelen] == tbl[i].index ){
		interface_ind = i;
		break;
	    }
	}
	if( i == tbl_size )
	    return MATCH_FAILED;
    } else {
	// DEBUGMSGTL(("mibII/shdsl", "Not exact, base_compare=%d\n",base_compare));
//	printf("ifIndex: Not exact, base_compare=%d\n",base_compare);
	// 1. OID for GETNEXT > vp base OID
	// 2. Index of GETNEXT > max index
	if( base_compare < 0 || ( !base_compare && *length<=vp->namelen) ){

	    //DEBUGMSGTL(("mibII/shdsl", "base_compare < 0\n",base_compare));
//	    printf("ifIndex: base_compare < 0\n");
	    memcpy((char*)name,(char*)newname,((int)vp->namelen+1)*sizeof(oid));
	    *length = vp->namelen + 1;
	    name[vp->namelen] = tbl[min_i].index;
	    interface_ind  = min_i;
	    
	}else if( !base_compare ){
//	    printf("ifIndex: (base_compare=0), name[vp->namelen]=%d\n",name[vp->namelen]);
	    min_i = 0;
	    min_ind = -1;
	    for( i=0;i<tbl_size;i++){
//		printf("ifIndex: Check index %d\n",tbl[i].index);
		if( tbl[i].index > name[vp->namelen] ){
//		    printf("ifIndex: ind > input ind\n");		
		    if( (min_ind < 0) || (min_ind > tbl[i].index) ){
//			printf("min_i = %d\n",i);
			min_i = i;
			min_ind = tbl[i].index;
		    }
		}
	    }
//	    printf("ifIndex: Result min_ind = %d, min_i=%d\n",min_ind,min_i);
	    if( min_ind>0 ){
//	    	printf("ifIndex: Apply new ind min_ind = %d, min_i=%d\n",
//								tbl[min_i].index,min_i);
		interface_ind = min_i;
		name[vp->namelen] = tbl[min_i].index;
	    }else{
//	    	printf("ifIndex: FAIL TO MATCH min_ind = %d, min_i=%d\n",min_ind,min_i);	    
		return MATCH_FAILED;
	    }
	} else
	    return MATCH_FAILED;
    }
    *write_method = 0;
    *var_len = sizeof(long); 
    dbg_oid("Result_oid ",name,*length);
    return name[vp->namelen];
}


/*
 * Span Configuration Group 
 */

u_char *
var_SpanConfEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    int iface;
    struct app_frame *fr1=NULL,*fr2=NULL;
    span_conf_payload *p;
    static char ConfProfile[SNMP_ADMIN_LEN];
    char *return_ptr = NULL;
    // Cacheing
    struct timeval tvcur;
    int tverr = 0;

    DEBUGMSGTL(("mibII/shdsl", "var_SpanConfEntry: *lenght=%d namelen=%d \n",*length,vp->namelen));
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\nexact= %d\n", exact));

    comm = init_comm();
    if(!comm){
        DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
        return NULL;
    }

    if ( ( iface=header_ifIndex(vp, name, length, exact, var_len,write_method) )
         == MATCH_FAILED || (*length != vp->namelen+1)){
//	DEBUGMSG(("mibII/shdsl", "SpanConf: MATCH_FAILED\n"));	
        printf("mibII/shdsl SpanConf: MATCH_FAILED\n");	
	goto exit;
    }
    printf("mibII/shdsl iface= %d, %s\n", iface,tbl[interface_ind].name);

    if( gettimeofday(&tvcur,NULL) ){
	printf("%s: tverr occured\n",__FUNCTION__);
	tverr = 1;
    }

    if( !spanconf_tbl[interface_ind] || tverr ||
	(tvcur.tv_sec - spanconf_tbl[interface_ind]->tv.tv_sec > CACHE_INT) ){

        p = (span_conf_payload*)comm_alloc_request(APP_SPAN_CONF,APP_GET,tbl[interface_ind].name,&fr1);
	if( !p ){
	    //DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	    printf("mibII/shdsl Cannot allocate application frame");
    	    goto exit;
	}
	fr2 = comm_request(comm,fr1);
        if( !fr2 ){
	    printf("Error requesting\n");
    	    goto exit;
	}
	p = (span_conf_payload*)comm_frame_payload(fr2);

        // Cache data
	if( !spanconf_tbl[interface_ind] )
	    spanconf_tbl[interface_ind] = malloc(sizeof(shdsl_spanconf));
	spanconf_tbl[interface_ind]->p = *p;
	spanconf_tbl[interface_ind]->tv = tvcur;
    }else{
	p = &spanconf_tbl[interface_ind]->p;
    }

    printf("mibII/shdsl Form response\n");	
    switch (vp->magic) {
    case CONF_NREPS:
	*var_len=sizeof(int);
	long_return=p->nreps;
	return_ptr = (u_char *) & long_return;
	break;
    case CONF_PRFL:
	strncpy(ConfProfile,p->conf_prof,SNMP_ADMIN_LEN);
	*var_len=strnlen(ConfProfile,SNMP_ADMIN_LEN);
	return_ptr = (u_char *)ConfProfile;	
	break;
    case CONF_ALARMPRFL:
	goto exit;
//!!	*var_len=strlen(ConfAlarmProfile);
//!!	return (u_char *)ConfAlarmProfile;	
    default:
        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_interfaces\n",
                    vp->magic));
	break;
    }

exit:
    if( fr1 )
	comm_frame_free(fr1);
    if( fr2 )
	comm_frame_free(fr2);
    comm_free(comm);
    comm = NULL;
    return return_ptr;
}


/*
 * Span Status Group 
 */

u_char *
var_SpanStatusEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    int iface;
    struct app_frame *fr1=NULL,*fr2=NULL;
    span_status_payload *p;
    char *return_ptr = NULL;
    // Cacheing 
    int tverr = 0;
    struct timeval tvcur;

//    DEBUGMSGTL(("mibII/shdsl", "var_SpanStatusEntry: \n"));
//    DEBUGMSGOID(("mibII/shdsl", name, *length));
//    DEBUGMSG(("mibII/shdsl", "\nexact= %d\n", exact));

    comm = init_comm();
    if(!comm){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
	return NULL;
    }

    if ( ( iface=header_ifIndex(vp, name, length, exact, var_len,write_method) )
	     == MATCH_FAILED || (*length != vp->namelen+1)){
//	DEBUGMSG(("mibII/shdsl", "SpanConf: MATCH_FAILED\n"));	
	printf("mibII/shdsl SpanConf: MATCH_FAILED\n");	
        goto exit;
    }
    printf("mibII/shdsl iface= %d, %s\n", iface,tbl[interface_ind].name);

    if( gettimeofday(&tvcur,NULL) ){
	printf("%s: tverr occured\n",__FUNCTION__);
	tverr = 1;
    }
    
    if( !spanstatus_tbl[interface_ind] || tverr ||
	    ((tvcur.tv_sec - spanstatus_tbl[interface_ind]->tv.tv_sec)>CACHE_INT) ){
        p = (span_status_payload*)comm_alloc_request(APP_SPAN_STATUS,APP_GET,tbl[interface_ind].name,&fr1);
	if( !p ){
	    //DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	    printf("mibII/shdsl Cannot allocate application frame");
	    goto exit;
	}
	fr2 = comm_request(comm,fr1);
	if( !fr2 ){
	    printf("Error requesting\n");
	    goto exit;
	}
	p = (span_status_payload*)comm_frame_payload(fr2);

        // Cache data
	if( !spanstatus_tbl[interface_ind] )
	    spanstatus_tbl[interface_ind] = malloc(sizeof(shdsl_spanstatus_elem));
	spanstatus_tbl[interface_ind]->p = *p;
        spanstatus_tbl[interface_ind]->tv = tvcur;
    }else{
	p = &spanstatus_tbl[interface_ind]->p;
    }
    
    //---- ack ----//
    switch (vp->magic) {
    case STATUS_NAVAILREPS:
	long_return= p->nreps;    
	if( p->nreps <0 )
	    long_return = 0;
	return_ptr = (u_char *) & long_return;
	break;
    case STATUS_MAXATTLRATE:
	long_return = p->max_lrate;
	return_ptr = (u_char *) & long_return;
	break;
    case STATUS_ACTLRATE:
	long_return = p->act_lrate;
	return_ptr = (u_char *) & long_return;
	break;
/*    case STATUS_TRNSMSNMODCUR:
//	long_return = *((unsigned char*)&cfg.annex);
	*var_len = sizeof(char);
	return (u_char *)&long_return;
*/    case STATUS_MAXATTPRATE:
	long_return = p->max_prate;
	return_ptr = (u_char *) & long_return;
	break;
    case STATUS_ACTPRATE:
	long_return = p->act_prate;
	return_ptr = (u_char *) & long_return;
	break;
    default:
	break;
    }
    
exit:
    if( fr1 )
	comm_frame_free(fr1);
    if( fr2 )
	comm_frame_free(fr2);
    comm_free(comm);
    comm = NULL;
    return return_ptr;

}

/*
 * Unit Inventory Group
 */

static int
header_unitIndex(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    int iface = 0;
    int length_bkp = *length;
    *write_method = 0;
    *var_len = sizeof(long);    // default to 'long' results //
    struct app_frame *fr1,*fr2;
    span_params_payload *p;
    // Cacheing
    struct timeval tvcur;
    int tverr = 0;
    
   // If OID is belong to Inventory Table
    if( (iface = header_ifIndex(vp, name, length,1/*exact = 1*/, var_len,write_method)) 
	    != MATCH_FAILED ){ 	// OID belongs to Invetory table and ifIdex is valid
//	printf("unitIndex: OID belong to Inv Table, if = %s\n",tbl[iface].name);
        
	if( gettimeofday(&tvcur,NULL) ){
	    printf("%s: tverr occured\n",__FUNCTION__);
	    tverr = 1;
	}

	if( ((tvcur.tv_sec - tbl[interface_ind].tv.tv_sec) > CACHE_INT) || tverr || 
		tbl[interface_ind].units < 0 ){
	    p = (span_params_payload*)comm_alloc_request(APP_SPAN_PARAMS,
			APP_GET,tbl[interface_ind].name,&fr1);
	    if( !p ){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
		printf("unitIndex: Cannot allocate application frame\n");
		return MATCH_FAILED;
	    }
	    fr2 = comm_request(comm,fr1);
	    if( !fr2 ){
		printf("unitIndex: Error requesting\n");
		return MATCH_FAILED;
	    }
	    p = (span_params_payload*)comm_frame_payload(fr2);
	    // Cache data
	    tbl[interface_ind].tv = tvcur;
	    tbl[interface_ind].units = p->units;
	    tbl[interface_ind].wires = p->loops;
	}


//	printf("unitIndex: Get params info for %s: units(%d) loops(%d)\n",tbl[iface].name,p->units,p->loops);

	if( exact ){ // Need exact MATCH
//	    printf("unitIndex: Exact match\n");
	    if( (*length >= vp->namelen+2) && 
		    (name[vp->namelen+1] > 0) && 
		    (name[vp->namelen+1]<= tbl[interface_ind].units) ){
//		    printf("unitIndex: Exact match - OK\n");
		return name[vp->namelen+1];
	    } else {
//		printf("unitIndex: Exact match - FAIL\n");
		return MATCH_FAILED;
	    }
	} else { // Need next regenerator index
	    if( *length >= vp->namelen+2 &&
		    (name[vp->namelen+1]+1 > 0) &&
		    (name[vp->namelen+1]+1 <= tbl[interface_ind].units) ){
//		printf("unitIndex: Next unit after %d\n",name[vp->namelen+1]);
		name[vp->namelen+1]++;
		return name[vp->namelen+1];
	    }else if( *length == vp->namelen + 1 ){
		name[vp->namelen+1] = stu_c;
//		printf("unitIndex:  First unit \n");
		*length = vp->namelen+2;
		return name[vp->namelen+1];
	    }
	}
    }
    
//    printf("unitIndex: OID does not belong to Inventory Table OR NO next regenerator\n");
    // OID does not belong to Inventory Table OR NO next regenerator 
    if( exact ){
//	printf("unitIndex: Exact match - seems failed\n");
	return MATCH_FAILED;
    }
    
    if( (iface=header_ifIndex(vp, name, length, exact, var_len,write_method)) 
		== MATCH_FAILED ){ // No next interface or In OID is lager than Inventory Table OIDs
//	printf("unitIndex: Search for next iface failed\n");
	return MATCH_FAILED;
    }

    if( ((tvcur.tv_sec - tbl[interface_ind].tv.tv_sec) > CACHE_INT) || tverr || 
		tbl[interface_ind].units < 0 ){
        p = (span_params_payload*)comm_alloc_request(APP_SPAN_PARAMS,
    	    	    APP_GET,tbl[interface_ind].name,&fr1);
	if( !p ){
    	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
    	    printf("unitIndex: Cannot allocate application frame\n");
    	    return MATCH_FAILED;
	}
	fr2 = comm_request(comm,fr1);
	if( !fr2 ){
    	    printf("unitIndex: Error requesting\n");
    	    return MATCH_FAILED;
	}
	p = (span_params_payload*)comm_frame_payload(fr2);
	//    printf("unitIndex: Get params info for %s: units(%d) loops(%d)\n",tbl[iface].name,p->units,p->loops);    
	tbl[interface_ind].units = p->units;
	tbl[interface_ind].wires = p->loops;
	tbl[interface_ind].tv = tvcur;
    }
	
    name[vp->namelen+1] = stu_c;
    *length = vp->namelen + 2;
//    printf("unitIndex: Result: unit #%d\n",name[vp->namelen+1]);
    return name[vp->namelen+1];
}


u_char *
var_InventoryEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    int unit;
    size_t length1;
    struct app_frame *fr1=NULL,*fr2=NULL;
    inventory_payload *p;
    resp_inventory *resp;
    char *return_ptr = NULL;

//    DEBUGMSGTL(("mibII/shdsl", "var_InventoryEntry: \n"));
//    DEBUGMSGOID(("mibII/shdsl", name, *length));
//    DEBUGMSG(("mibII/shdsl", "\nexact= %d\n", exact));

    comm = init_comm();
    if(!comm){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
	return NULL;
    }

    if ( ( unit = header_unitIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED ){
	printf("var_InventoryEntry: Cannot find unit\n");
	goto exit;
    }
    printf("var_InventoryEntry: unit = %d\n",unit);

// ------------- Need cacheing ------------------//

	p = (inventory_payload*)comm_alloc_request(APP_INVENTORY,APP_GET,
		tbl[interface_ind].name,&fr1);
	if( !p ){
	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	    printf("var_InventoryEntry: Cannot allocate application frame\n");
	    goto exit;
	}

	p->unit = unit;
	fr2 = comm_request(comm,fr1);
	if( !fr2 && exact ){
	    printf("var_InventoryEntry: Error requesting\n");
	    goto exit;
	}
    
	while( !fr2 ){
	    if ( ( unit = header_unitIndex(vp,name,length,exact,var_len,write_method) )
		    == MATCH_FAILED ){
		printf("var_InventoryEntry: Cannot find unit\n");
		goto exit;
	    }
	    printf("var_InventoryEntry: unit = %d\n",unit);

	    p = (inventory_payload*)comm_alloc_request(APP_INVENTORY,APP_GET,
		tbl[interface_ind].name,&fr1);
	    if( !p ){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
		printf("var_InventoryEntry: Cannot allocate application frame\n");
		goto exit;
	    }
	    p->unit = unit;
	
	    fr2 = comm_request(comm,fr1);
	}
    
	if( !fr2 ){
	    printf("var_InventoryEntry: Error requesting\n");
	    goto exit;
	}
        p = (inventory_payload*)comm_frame_payload(fr2);

//~~~~~~~~~~~~~~~~~~ Need cacheing ~~~~~~~~~~~~~~~~~~~~~~~~`//

    resp = &p->inv;
    printf("var_InventoryEntry: Result: magic=%d if=%s, unit = %d\n",vp->magic,tbl[interface_ind].name,unit);

    //---- ack ----//
    switch (vp->magic) {
    case INV_VID:
	*var_len = sizeof(resp->ven_id);
	memset(return_buf,0,*var_len);
	strncpy(return_buf,resp->ven_id,*var_len);    
	return_ptr = (u_char *)return_buf;
	break;
    case INV_VMODELNUM:
	*var_len = sizeof(resp->ven_model);
	memset(return_buf,0,*var_len);
	strncpy(return_buf,resp->ven_model,*var_len);    
	return_ptr = (u_char *)return_buf;
	break;
    case INV_VSERNUM:
	*var_len = sizeof(resp->ven_serial);
	memset(return_buf,0,*var_len);
	strncpy(return_buf,resp->ven_serial,*var_len);    
	return_ptr = (u_char *)return_buf;
	break;
    case INV_VEOCSV:
	long_return=p->eoc_softw_ver;
	return_ptr = (u_char *)&long_return;
	break;
    case INV_STANDARDV:
	long_return = resp->shdsl_ver;
	return_ptr = (u_char *)&long_return;
	break;
    case INV_VLISTNUM:
	*var_len = strlen(resp->ven_lst);
	strncpy(return_buf,resp->ven_lst,*var_len);
	return_ptr = (u_char *)return_buf;
	break;
    case INV_VISSUENUM:
	*var_len = strlen(resp->ven_issue);
	strncpy(return_buf,resp->ven_issue,*var_len);
	return_ptr = (u_char *)return_buf;
	break;
    case INV_VSOFTWV:
	*var_len = strlen(resp->softw_ver);
	strncpy(return_buf,resp->softw_ver,*var_len);
	return_ptr = (u_char *)return_buf;
	break;
    case INV_EQCODE:
	*var_len = strlen(resp->unit_id_code);
	strncpy(return_buf,resp->unit_id_code,*var_len);
	return_ptr = (u_char *)return_buf;
	break;
    case INV_VOTHER:
	*var_len = strlen(resp->other);
	strncpy(return_buf,resp->other,*var_len);    
	return_ptr = (u_char *)return_buf;
	break;
/*    case INV_TRNSMODECPB:
	*var_len = sizeof(info->TransModeCpb);
	long_return=info->TransModeCpb;
	return (u_char *)&long_return;
*/    }

exit:
    if( fr1 )
	comm_frame_free(fr1);
    if( fr2 )
	comm_frame_free(fr2);
    comm_free(comm);
    comm = NULL;
    return return_ptr;
}

/*
 * ------------ Segment Endpoint Group --------------------
 */
 
 
/*
 * header_endpIndex:
 * 	Defines propriate endpoint index for incoming OID
 */

static int 
header_endpIndex(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method )
{
    int unit = 0;
    int length_bkp = *length;
    *write_method = 0;
    *var_len = sizeof(long);    // default to 'long' results //

    if ( ( unit = header_unitIndex(vp,name,length, 1 /*exact = 1*/,var_len,write_method) )
	    != MATCH_FAILED ){
//	printf("endpIndex: uinit matched: %d\n",unit);
	unit_index = unit;

	if( exact ){ // Need exact MATCH
	    if( (*length >= vp->namelen+3) ){
		switch( unit ){
		case stu_c:
		    if( name[vp->namelen+2] != (cust_side+1) )
			return MATCH_FAILED;
		    else
			return name[vp->namelen+2];
		    break;
		case stu_r:
		    if( name[vp->namelen+2] != (net_side+1) )		
			return MATCH_FAILED;
		    else
			return name[vp->namelen+2];
		    break;
		case sru1:
		case sru2:
		case sru3:
		case sru4:
		case sru5:
		case sru6:
		case sru7:
		case sru8:
		    if( name[vp->namelen+2] == (cust_side+1) || 
			name[vp->namelen+2] == (net_side+1) )
			return name[vp->namelen+2];
		    else
			return MATCH_FAILED;
		default:
		    return MATCH_FAILED;
		}
	    } else
		return MATCH_FAILED;
	} else { // Nonexact match
//	    printf("endpIndex: nonexact match: unit(%d) side(%d)\n",unit,name[vp->namelen+2]);
	    if( unit >= sru1 && unit <=sru8 ){
		if( *length >= vp->namelen+3 ){
		    if( name[vp->namelen+2] == (net_side+1) ){
			name[vp->namelen+2] = (cust_side+1);
			return name[vp->namelen+2];
		    }
		}else if( *length == vp->namelen+2 ){
		    name[vp->namelen+2] == (net_side+1);
		    *length = vp->namelen+3;
    		    return name[vp->namelen+2];	
		}
	    }
	}
    }
	
    // OID does not belong to Inventory Table OR NO next regenerator 
    if( exact ){
	return MATCH_FAILED;
    }
//    printf("endpIndex: SWITCH to next unit\n");
    if( (unit=header_unitIndex(vp, name, length, exact, var_len,write_method)) 
	    == MATCH_FAILED ){ // No next interface or In OID is lager than Inventory Table OIDs
	return MATCH_FAILED;
    }
    unit_index = unit;
//    printf("endpIndex: next unit = %d\n",unit);
    switch( unit ){
    case stu_c:
	name[vp->namelen+2] = (cust_side+1);
	break;
    case stu_r:
	name[vp->namelen+2] = (net_side+1);    
	break;
    case sru1:
    case sru2:
    case sru3:
    case sru4:
    case sru5:
    case sru6:
    case sru7:
    case sru8:
	name[vp->namelen+2] = (net_side+1);
	break;
    default:
	return MATCH_FAILED;
    }
    *length = vp->namelen + 3;
    return name[vp->namelen+2];
}


/*
 * header_wirePairIndex:
 * 	Defines propriate Wire pair index for incoming OID
 *	(by now only 1 pair supported)
 */

static int 
header_wirePairIndex(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method )
{
    int endp = 0;
    *write_method = 0;
    *var_len = sizeof(long);    // default to 'long' results //
    struct app_frame *fr1,*fr2;
    span_params_payload *p;
    // Cacheing
    struct timeval tvcur;
    int tverr = 0;

// printf("----------- WirePair: start ------------\n");

/*
DEBUGMSGTL(("mibII/shdsl", "Input OID: "));
DEBUGMSGOID(("mibII/shdsl", name, *length));		    
DEBUGMSG(("mibII/shdsl", "\n"));

DEBUGMSGTL(("mibII/shdsl", "Local OID: "));
DEBUGMSGOID(("mibII/shdsl", vp->name, vp->namelen));		    
DEBUGMSG(("mibII/shdsl", "\n"));
*/

    if ( ( endp = header_endpIndex(vp,name,length, 1 /*exact = 1*/,var_len,write_method) )
	    != MATCH_FAILED ){
    
//	printf("wirePair: exact endpoint = %d\n",endp);
	endp_index = endp;
	tverr = gettimeofday(&tvcur,NULL);
	if( tverr || tbl[interface_ind].wires < 0 ||
	    (tvcur.tv_sec - tbl[interface_ind].tv.tv_sec > CACHE_INT ) ){ 
	    p = (span_params_payload*)comm_alloc_request(APP_SPAN_PARAMS,
			APP_GET,tbl[interface_ind].name,&fr1);
	    if( !p ){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
		printf("wirePair: Cannot allocate application frame\n");
		return MATCH_FAILED;
	    }
	    fr2 = comm_request(comm,fr1);
	    if( !fr2 ){
		printf("wirePair: Error requesting\n");
		return MATCH_FAILED;
	    }
	    p = (span_params_payload*)comm_frame_payload(fr2);
	    tbl[interface_ind].units = p->units;
	    tbl[interface_ind].wires = p->loops;
	    tbl[interface_ind].tv = tvcur;
	}
	    
//	printf("wirePair: Get params info for %s: units(%d) loops(%d)\n",tbl[interface_ind].name,p->units,p->loops);

	if( exact ){ // Need exact MATCH
//	    printf("wirePair: Exact Match\n");
	    if( (*length >= vp->namelen+4) && 
		(name[vp->namelen+3] > 0) && (name[vp->namelen+3] <= tbl[interface_ind].wires) ){
		return name[vp->namelen+3];
	    } else
		return MATCH_FAILED;
	} else { // Nonexact match
	    // Because we have only one wire pair - we change it only 
	    // if field is empty
	    if( *length == vp->namelen+3 ){
	        name[vp->namelen+3] = 1; // first pair
	        *length = vp->namelen+4;
		return name[vp->namelen+3];
	    }else{
		if( name[vp->namelen+3] <= 0 )
		    return MATCH_FAILED;
		if( name[vp->namelen+3]+1 <= tbl[interface_ind].wires ){
		    *length = vp->namelen+4;
		    return (++name[vp->namelen+3]);
		}
	    }
	}
    }
	
    if( exact ){
	return MATCH_FAILED;
    }
    
    if( (endp = header_endpIndex(vp, name, length, exact, var_len,write_method)) 
	    == MATCH_FAILED ){ // No next interface or In OID is lager than Inventory Table OIDs
	return MATCH_FAILED;
    }

//    printf("wirePair: nonexact endpoint = %d\n",endp);
    endp_index = endp;

    if( tverr || tbl[interface_ind].wires < 0 ||
        (tvcur.tv_sec - tbl[interface_ind].tv.tv_sec > CACHE_INT ) ){ 
        p = (span_params_payload*)comm_alloc_request(APP_SPAN_PARAMS,
    		APP_GET,tbl[interface_ind].name,&fr1);
        if( !p ){
    	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	    printf("wirePair: Cannot allocate application frame\n");
	    return MATCH_FAILED;
	}
	fr2 = comm_request(comm,fr1);
	if( !fr2 ){
	    printf("wirePair: Error requesting\n");
	    return MATCH_FAILED;
	}
	p = (span_params_payload*)comm_frame_payload(fr2);
	tbl[interface_ind].units = p->units;
	tbl[interface_ind].wires = p->loops;
	tbl[interface_ind].tv = tvcur;
    }

    name[ vp->namelen+3] = 1; // First pair
    *length = vp->namelen + 4;
//    DEBUGMSGTL(("mibII/shdsl", "Result: PAIR #%d\n",name[vp->namelen+3]));
//    printf("wirePair: returning\n");
    return name[vp->namelen+3];
}


/*
 * Segment Endpoint Current Status/Performance Group
 */

u_char *
var_EndpointCurrEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    struct app_frame *fr1=NULL,*fr2=NULL;
    endp_cur_payload *p;
    char *return_ptr = NULL;
    int pair,endp;

    comm = init_comm();
    if(!comm){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
	return NULL;
    }

    printf("EndpointCurStatEntry: start\n");

/*
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\n"));    
*/


// ------------------------------ Obtain requested information ------------------------------- //

    if ( ( pair = header_wirePairIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED )
        goto exit;
    
    printf("Result : if(%s) unit(%d) side(%d) pair(%d)\n",tbl[interface_ind].name,unit_index,endp_index,pair);

    p = (endp_cur_payload*)comm_alloc_request(APP_ENDP_CUR,APP_GET,
	    tbl[interface_ind].name,&fr1);
    if( !p ){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	printf("var_InventoryEntry: Cannot allocate application frame\n");
	goto exit;
    }
    p->unit = unit_index;
    p->side = endp_index-1;
    p->loop = pair-1;
    fr2 = comm_request(comm,fr1);
    printf("Request if(%s) unit(%d) side(%d) loop(%d)\n",
	tbl[interface_ind].name,p->unit,p->side,p->loop);

    
    if( !fr2 && exact ){
	printf("var_InventoryEntry: error requesting\n");
	goto exit;
    }
    
    while( !fr2 ){
	if ( ( pair = header_wirePairIndex(vp,name,length,exact,var_len,write_method) )
		== MATCH_FAILED )
    	    goto exit;
    
	printf("Result (rep): if(%s) unit(%d) side(%d) pair(%d)\n",tbl[interface_ind].name,unit_index,endp_index,pair);

	p = (endp_cur_payload*)comm_alloc_request(APP_ENDP_CUR,APP_GET,
	    tbl[interface_ind].name,&fr1);
	if( !p ){
	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	    printf("var_InventoryEntry: Cannot allocate application frame\n");
	    goto exit;
	}
	p->unit = unit_index;
	p->side = endp_index-1;
	p->loop = pair-1;
	fr2 = comm_request(comm,fr1);

	printf("Request (rep): if(%s) unit(%d) side(%d) loop(%d)\n",
		tbl[interface_ind].name,p->unit,p->side,p->loop);

    }

    if( !fr2 ){
	printf("var_InventoryEntry: error requesting\n");
	goto exit;
    }
    
    p = (endp_cur_payload*)comm_frame_payload(fr2);
    
// ------------------------------ Return requested information ------------------------------- //

    //---- ack ----//
    switch (vp->magic) {
    case ENDP_STAT_CUR_ATN:
	long_return = p->cur_attn;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_SNRMGN:
	long_return = p->cur_snr;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_STATUS:
	break;
    case ENDP_STAT_CUR_ES:
	long_return = (u_long)p->total.es;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_SES:
	long_return = (u_long)p->total.ses;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_CRC:
	long_return = (u_long)p->total.crc;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_LOSWS:
	long_return = (u_long)p->total.losws;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_UAS:
	long_return = (u_long)p->total.uas;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_15MEL:
	long_return = p->cur_15m_elaps;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_ES:
	long_return = p->cur15min.es;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_SES:
	long_return = p->cur15min.ses;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_CRC:
	long_return = p->cur15min.crc;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_LOSWS:
	long_return = p->cur15min.losws;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_UAS:
	long_return = p->cur15min.uas;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1DEL:
	long_return = p->cur_1d_elaps;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_ES:
	long_return = p->cur1day.es;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_SES:
	long_return = p->cur1day.ses;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_CRC:
	long_return = p->cur1day.crc;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_LOSWS:
	long_return = p->cur1day.losws;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_UAS:
	long_return = p->cur1day.uas;
	return_ptr = (char*)&long_return;
	break;
    default:
	break;
    }


exit:
    if( fr1 )
	comm_frame_free(fr1);
    if( fr2 )
	comm_frame_free(fr2);
    comm_free(comm);
    comm = NULL;
    return return_ptr;
}

// ----------------- Perfomance/Status intervals ------------------//

static int 
header_interval15Index(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method,app_ids msg_id)
{
    int wire = 0;
    *write_method = 0;
    *var_len = sizeof(long);    // default to 'long' results //
    struct app_frame *fr1,*fr2;
    endp_int_payload *p;
    int int_num;
    printf("\n----------------------- interval15Index ----------------------\n");
    printf("Input: if(%d) unit(%d) side(%d) pair(%d) int(%d)\n",
	    name[vp->namelen],name[vp->namelen+1],name[vp->namelen+2],
	    name[vp->namelen+3],name[vp->namelen+4]);


// printf("----------- WirePair: start ------------\n");
    //-------------- Try requested wire_pair ----------------//
    if ( ( wire = header_wirePairIndex(vp,name,length, 1 /*exact = 1*/,var_len,write_method) )
	    != MATCH_FAILED ){
    
	printf("interval15Index: exact wire = %d\n",wire);
	wire_index = wire;
	
	if( exact ){
	    if( *length != vp->namelen+5 )
		return MATCH_FAILED;
	    p = (endp_int_payload*)comm_alloc_request(msg_id,APP_GET,
			tbl[interface_ind].name,&fr1);
	    if( !p ){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
		printf("interval15Index: Cannot allocate application frame\n");
		return MATCH_FAILED;
	    }
	    int_num = name[vp->namelen+4];
	} else {
	    if( *length < vp->namelen+4 ){
		return MATCH_FAILED;
	    } else if( *length == vp->namelen+3 ){
		int_num = 1;
	    }else{
//		printf("interval15Index: Shift interval: %d to %d"
		int_num = name[vp->namelen+4]+1;
	    }
	}

	p = (endp_int_payload*)comm_alloc_request(msg_id,APP_GET,
		tbl[interface_ind].name,&fr1);
	if( !p ){
	    DEBUGMSGTL(("mibII/hdsl2Shdsl","interval15Index: Cannot allocate application frame"));
	    printf("interval15Index: Cannot allocate application frame\n");
	    return MATCH_FAILED;
	}
	    
	p->unit = unit_index;
	p->side = endp_index-1;
	p->loop = wire_index-1;
	p->int_num = int_num;

	printf("Output: unit(%d) side(%d) pair(%d) int(%d)\n",
		p->unit,p->side,p->loop,p->int_num);
	
	fr2 = comm_request(comm,fr1);
	if( fr2 ){
	    p = (endp_int_payload*)comm_frame_payload(fr2);
	    //	printf("wirePair: Get params info for %s: units(%d) loops(%d)\n",tbl[interface_ind].name,p->units,p->loops);
	    perf_int = p->cntrs;
	    name[vp->namelen+4] = int_num;
	    *length = vp->namelen+5;
	    return name[vp->namelen+4];
	}
    }
    
    if( exact ){
	return MATCH_FAILED;
    }
    
    //-------------- Switch to next wire_pair ----------------//
    if ( ( wire = header_wirePairIndex(vp,name,length, exact,var_len,write_method) )
	    == MATCH_FAILED ){
	return MATCH_FAILED;
    }
    wire_index = wire;
    printf("interval15Index: nonexact wire = %d\n",wire);

    p = (endp_int_payload*)comm_alloc_request(msg_id,APP_GET,
    		tbl[interface_ind].name,&fr1);
    if( !p ){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","interval15Index: Cannot allocate application frame"));
	printf("interval15Index: Cannot allocate application frame\n");
	return MATCH_FAILED;
    }

    p->unit = unit_index;
    p->side = endp_index-1;
    p->loop = wire_index-1;
    p->int_num = 1;

    printf("Output: unit(%d) side(%d) pair(%d) int(%d)\n",
		p->unit,p->side,p->loop,p->int_num);

    fr2 = comm_request(comm,fr1);
    if( !fr2 ){
    	DEBUGMSGTL(("mibII/hdsl2Shdsl","interval15Index: Error requesting\n"));
    	printf("interval15Index: Error requesting\n");
	return MATCH_FAILED;
    }

    printf("Request successfull\n");
    p = (endp_int_payload*)comm_frame_payload(fr2);
    //	printf("wirePair: Get params info for %s: units(%d) loops(%d)\n",tbl[interface_ind].name,p->units,p->loops);
    perf_int = p->cntrs;
    name[vp->namelen+4] = p->int_num;
    *length = vp->namelen+5;
    return name[vp->namelen+4];
}



/*
 * Segment Endpoint 15-Minute Interval Status/Performance Group 
 */

u_char *
var_15MinIntervalEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    struct app_frame *fr1=NULL,*fr2=NULL;
    endp_int_payload *p15,*p1d;
    char *return_ptr = NULL;
    int int_num;

    comm = init_comm();
    if(!comm){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
	return NULL;
    }


/*
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\n"));    
*/

    if ( (int_num = header_interval15Index(vp,name,length,exact,var_len,write_method,APP_ENDP_15MIN) )
	    == MATCH_FAILED )
        goto exit;
    
    printf("Result : if(%s) unit(%d) side(%d) pair(%d) int(%d)\n",tbl[interface_ind].name,unit_index,endp_index,wire_index,int_num);

    //---- ack ----//
    switch (vp->magic) {
    case ENDP_15M_ES:
	long_return = (u_long)perf_int.es;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_15M_SES:
	long_return = (u_long)perf_int.ses;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_15M_CRC:
	long_return = (u_long)perf_int.crc;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_15M_LOSWS:
	long_return = (u_long)perf_int.losws;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_15M_UAS:
	long_return = (u_long)perf_int.uas;
	return_ptr = (u_char*)&long_return;
	break;
    default:
	break;
    }


exit:
    if( fr1 )
	comm_frame_free(fr1);
    if( fr2 )
	comm_frame_free(fr2);
    comm_free(comm);
    comm = NULL;
    return return_ptr;
}



/*
 * Segment Endpoint 1-Day Interval Status/Performance Group
 */
/*
u_char *
var_EndpointCurrEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    struct app_frame *fr1=NULL,*fr2=NULL;
    endp_cur_payload *p;
    endp_int_payload *p15,*p1d;
    char *return_ptr = NULL;
    int pair,endp;

    comm = init_comm();
    if(!comm){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
	return NULL;
    }

    printf("EndpointCurStatEntry: start\n");

/*
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\n"));    
*//*

    if ( ( pair = header_wirePairIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED )
        goto exit;
    
    printf("Result : if(%s) unit(%d) side(%d) pair(%d)\n",tbl[interface_ind].name,unit_index,endp_index,pair);

    p = (endp_cur_payload*)comm_alloc_request(APP_ENDP_CUR,APP_GET,
	    tbl[interface_ind].name,&fr1);
    if( !p ){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	printf("var_InventoryEntry: Cannot allocate application frame\n");
	goto exit;
    }
    p->unit = unit_index;
    p->side = endp_index-1;
    p->loop = pair-1;
    printf("Request if(%s) unit(%d) side(%d) loop(%d)\n",
	tbl[interface_ind].name,p->unit,p->side,p->loop);

    fr2 = comm_request(comm,fr1);
    if( !fr2 ){
	printf("Error requesting\n");
	goto exit;
    }
    p = (endp_cur_payload*)comm_frame_payload(fr2);


    //---- ack ----//
    switch (vp->magic) {
    case ENDP_STAT_CUR_ATN:
	long_return = p->cur_attn;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_SNRMGN:
	long_return = p->cur_snr;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_STATUS:
	break;
    case ENDP_STAT_CUR_ES:
	long_return = (u_long)p->total.es;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_SES:
	long_return = (u_long)p->total.ses;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_CRC:
	long_return = (u_long)p->total.crc;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_LOSWS:
	long_return = (u_long)p->total.losws;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_UAS:
	long_return = (u_long)p->total.uas;
	return_ptr = (u_char*)&long_return;
	break;
    case ENDP_STAT_CUR_15MEL:
	long_return = p->cur_15m_elaps;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_ES:
	long_return = p->cur15min.es;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_SES:
	long_return = p->cur15min.ses;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_CRC:
	long_return = p->cur15min.crc;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_LOSWS:
	long_return = p->cur15min.losws;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_15M_UAS:
	long_return = p->cur15min.uas;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1DEL:
	long_return = p->cur_1d_elaps;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_ES:
	long_return = p->cur1day.es;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_SES:
	long_return = p->cur1day.ses;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_CRC:
	long_return = p->cur1day.crc;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_LOSWS:
	long_return = p->cur1day.losws;
	return_ptr = (char*)&long_return;
	break;
    case ENDP_STAT_CUR_1D_UAS:
	long_return = p->cur1day.uas;
	return_ptr = (char*)&long_return;
	break;
    default:
	break;
    }


exit:
    if( fr1 )
	comm_frame_free(fr1);
    if( fr2 )
	comm_frame_free(fr2);
    comm_free(comm);
    comm = NULL;
    return return_ptr;
}



/*
 * Segment Endpoint Configuration Group
 *//*
u_char *
var_EndpointConfEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    shdsl_unit_t Info, *info = &Info;
    int pair;

//    DEBUGMSGTL(("mibII/shdsl", "\n-----------------------START----------------------------\n"
//			"var_EndpointConfEntry. exact= %d\n",exact));
//    DEBUGMSGOID(("mibII/shdsl", name, *length));
//    DEBUGMSG(("mibII/shdsl", "\n"));    
    
    if ( ( pair = header_wirePairIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED )
        return NULL;






    struct app_frame *fr1,*fr2;
    char *b;
    endp_int_payload *p;
    

    p = (endp_int_payload*)comm_alloc_request(APP_ENDP_15MIN,APP_GET,"dsl0",&fr1);
    if( !p ){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	return;
    }
    p->unit = stu_c;
    p->side = net_side;
    p->loop = 0;
    p->int_num = 73;

    fr2 = comm_request(c,fr1);
    if( !fr2 ){
	printf("Error requesting\n");
	return -1;
    }
    p = (endp_int_payload*)comm_frame_payload(fr2);
    print_int_payload(p,"1day");    








    
//    DEBUGMSGTL(("mibII/shdsl", "Result pair = %d\n-------------------END------------------------\n",pair));
    //---- ack ----//
    switch (vp->magic) {
    case ENDP_CONF_PROF:
	strcpy(return_buf,"DEFAULT");    
	*var_len = strlen(return_buf);
	return (u_char *)return_buf;
    }    
    return NULL;
}


/*
 *
 * ---------------------- Maintenance Group  ------------------------
 *
 *//*

u_char *
var_EndpointMaintEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    shdsl_endp_maint_t Info, *info = &Info;
    int pair;
/*
    DEBUGMSGTL(("mibII/shdsl", "\n-----------------------START----------------------------\n"
			"var_EndpointConfEntry. exact= %d\n",exact));
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\n"));    
*//*    
    if ( ( pair = header_endpIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED )
        return NULL;



    struct app_frame *fr1,*fr2;
    char *b;
    endp_int_payload *p;
    

    p = (endp_int_payload*)comm_alloc_request(APP_ENDP_15MIN,APP_GET,"dsl0",&fr1);
    if( !p ){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	return;
    }
    p->unit = stu_c;
    p->side = net_side;
    p->loop = 0;
    p->int_num = 73;

    fr2 = comm_request(c,fr1);
    if( !fr2 ){
	printf("Error requesting\n");
	return -1;
    }
    p = (endp_int_payload*)comm_frame_payload(fr2);
    print_int_payload(p,"1day");    







    
    eocd_endp_maint(interface_ind,unit_index,&eocd,info);    
    
//    DEBUGMSGTL(("mibII/shdsl", "Result pair = %d\n-------------------END------------------------\n",pair));
    //---- ack ----//
    switch (vp->magic) {
    case ENDP_MAINT_LOOPBACK:
	long_return= info->lpb_cfg;
	return (u_char *) & long_return;
    case ENDP_MAINT_TIPRINGREV:
	long_return= info->ring_rev;
	return (u_char *) & long_return;
    case ENDP_MAINT_PWRBACKOFF:
	long_return= info->pwr_backoff;
	return (u_char *) & long_return;
    case ENDP_MAINT_SOFTRESTART:
	long_return= info->soft_rst;
	return (u_char *) & long_return;
    }
    return NULL;
}

u_char *
var_UnitMaintEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    shdsl_unit_maint_t Info, *info = &Info;
    int unit;

//    DEBUGMSGTL(("mibII/shdsl", "\n-----------------------START----------------------------\n"
//			"var_UnitMaintEntry. exact= %d\n",exact));
//    DEBUGMSGOID(("mibII/shdsl", name, *length));
//    DEBUGMSG(("mibII/shdsl", "\n"));    
    
    if ( ( unit = header_unitIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED )
        return NULL;



    struct app_frame *fr1,*fr2;
    char *b;
    endp_int_payload *p;
    

    p = (endp_int_payload*)comm_alloc_request(APP_ENDP_15MIN,APP_GET,"dsl0",&fr1);
    if( !p ){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	return;
    }
    p->unit = stu_c;
    p->side = net_side;
    p->loop = 0;
    p->int_num = 73;

    fr2 = comm_request(c,fr1);
    if( !fr2 ){
	printf("Error requesting\n");
	return -1;
    }
    p = (endp_int_payload*)comm_frame_payload(fr2);
    print_int_payload(p,"1day");    





    
    eocd_unit_maint(interface_ind,unit_index,&eocd,info);    
    
//    DEBUGMSGTL(("mibII/shdsl", "Result unit = %d\n-------------------END------------------------\n",unit));
    //---- ack ----//
    switch (vp->magic) {
    case ENDP_MAINT_LOOPBACK:
	long_return= info->lpb_to;
	return (u_char *) & long_return;
    case ENDP_MAINT_TIPRINGREV:
	long_return= info->pwr_src;
	return (u_char *) & long_return;
    }
    return NULL;
}


/*
 *
 * ---------------------- PROFILES ------------------- 
 *
 */


/*
 * header_wirePairIndex:
 * 	Defines propriate Wire pair index for incoming OID
 *	(by now only 1 pair supported)
 */
/*
char prof_names[256][32];

static int 
header_confProfIndex(struct variable *vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method )
{
    oid newname[MAX_OID_LEN];
    oid tmpname[MAX_OID_LEN];    
    int newlen=0, tmplen=0, sublen=0;
    int i, k, cnt, min_i=-1;
    int result;
    
    
    *write_method = 0;
    *var_len = sizeof(long);    // default to 'long' results //
/*
DEBUGMSGTL(("mibII/shdsl", "\n---------------------------------\nheader_confProfIndex:\n"));
DEBUGMSGTL(("mibII/shdsl", "Input OID: "));
DEBUGMSGOID(("mibII/shdsl", name, *length));		    
DEBUGMSG(("mibII/shdsl", "\n"));

DEBUGMSGTL(("mibII/shdsl", "Local OID: "));
DEBUGMSGOID(("mibII/shdsl", vp->name, vp->namelen));		    
DEBUGMSG(("mibII/shdsl", "\n"));

*//*    

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    newlen = vp->namelen;
    memcpy((char *) tmpname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));
    tmplen = vp->namelen;




    struct app_frame *fr1,*fr2;
    char *b;
    endp_int_payload *p;
    

    p = (endp_int_payload*)comm_alloc_request(APP_ENDP_15MIN,APP_GET,"dsl0",&fr1);
    if( !p ){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	return;
    }
    p->unit = stu_c;
    p->side = net_side;
    p->loop = 0;
    p->int_num = 73;

    fr2 = comm_request(c,fr1);
    if( !fr2 ){
	printf("Error requesting\n");
	return -1;
    }
    p = (endp_int_payload*)comm_frame_payload(fr2);
    print_int_payload(p,"1day");    





    cnt = prof_conf_names(prof_names);
    
    for(i=0;i<cnt;i++){
//	DEBUGMSGTL(("mibII/shdsl", "Analyse Profile: %s\n",prof_names[i]));
	sublen = strlen(prof_names[i]);
	for(k=0;k<sublen;k++)
	    newname[newlen+k] = prof_names[i][k];
//	DEBUGMSGTL(("mibII/shdsl", "Builded OID: "));
//	DEBUGMSGOID(("mibII/shdsl", newname, newlen+sublen));		    
//	DEBUGMSG(("mibII/shdsl", "\n"));
	
	result = snmp_oid_compare(newname,newlen+sublen,name,*length);
//	DEBUGMSGTL(("mibII/shdsl", "Result of comparision: %d\n",result));
	if( exact && !result ){
//	    DEBUGMSGTL(("mibII/shdsl", "Exact ok\n"));
	    return i;
	} else if( !exact && result>0 ){
//	    DEBUGMSGTL(("mibII/shdsl", "NonExact\n"));
	    if( tmplen == vp->namelen ){
//		DEBUGMSGTL(("mibII/shdsl", "First tmpname init\n"));
		memcpy((char *)tmpname, (char *)newname,(newlen+sublen)*sizeof(oid) );
		tmplen = newlen+sublen;
/*		DEBUGMSGTL(("mibII/shdsl", "tmpname OID: "));
		DEBUGMSGOID(("mibII/shdsl", tmpname, tmplen));		    
		DEBUGMSG(("mibII/shdsl", "\n"));
*//*		min_i = i;
	    } else {
/*		DEBUGMSGTL(("mibII/shdsl", "Compare newname with tmpname\n"));	    
		DEBUGMSGTL(("mibII/shdsl", "tmpname OID: "));
		DEBUGMSGOID(("mibII/shdsl", tmpname, tmplen));		    
		DEBUGMSG(("mibII/shdsl", "\n"));
*//*	    	result = snmp_oid_compare(newname,newlen+sublen,tmpname,tmplen);
//		DEBUGMSGTL(("mibII/shdsl", "Result of comparision: %d\n",result));
		if( result < 0 ){
//		    DEBUGMSGTL(("mibII/shdsl", "Save new candidate in tmpname\n"));		
		    memcpy((char *)tmpname, (char *)newname,(newlen+sublen)*sizeof(oid) );		    
		    tmplen = newlen+sublen;
		    min_i = i;
		}
	    }
	}
    }
    if(min_i < 0){
	return MATCH_FAILED;
    }
    memcpy((char *)name, (char *)tmpname,(tmplen)*sizeof(oid) );		    
    *length = tmplen;
/*
    DEBUGMSGTL(("mibII/shdsl", "Result OID: "));
    DEBUGMSGOID(("mibII/shdsl", name, *length));		    
    DEBUGMSG(("mibII/shdsl", "\n"));
*//*
    
    return min_i;
}



u_char *
var_SpanConfProfEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    int prof_ind;
    shdsl_conf_prof_t Info, *info=&Info;
    
    if ( ( prof_ind = header_confProfIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED )
        return NULL;

    eocd_init(&eocd);
    if( prof_conf_getrow(prof_ind,&eocd,info) ){
//	DEBUGMSGTL(("mibII/shdsl", "Error gettong conf row\n"));
	return NULL;
    }
    
//    DEBUGMSGTL(("mibII/shdsl", "Result prof_ind = %d\n-------------------END------------------------\n",prof_ind));
    //---- ack ----//
    switch (vp->magic) {
    case CONF_WIRE_IFACE:
	long_return= info->wire_if;
	return (u_char *) & long_return;
    case CONF_MIN_LRATE:
	long_return = info->min_lrate;
	return (u_char *) & long_return;
    case CONF_MAX_LRATE:
	long_return = info->max_lrate;
	return (u_char *) & long_return;
    case CONF_PSD:
	long_return= info->psd;
	return (u_char *) & long_return;
    case CONF_TRNSM_MODE:
	*var_len = sizeof(char);
	long_return = *((unsigned char*)&info->transm_mode);
	return (u_char *) & long_return;
    case CONF_REM_ENABLE:
	long_return= info->rem_conf;
	return (u_char *) & long_return;
    case CONF_PWR_FEED:
	long_return= info->pwr_feed;
	return (u_char *) & long_return;
    case CONF_CURR_DOWN:
	long_return= info->cur_cond_down;
	return (u_char *) & long_return;
    case CONF_WORST_DOWN:
	long_return= info->worst_case_down;
	return (u_char *) & long_return;
    case CONF_CURR_UP:
	long_return= info->cur_cond_up;
	return (u_char *) & long_return;
    case CONF_WORST_UP:
	long_return= info->worst_case_up;
	return (u_char *) & long_return;
    case CONF_USED_MARG:
	*var_len = sizeof(char);
	long_return= *((unsigned char*)&info->used_margins);
	return (u_char *) & long_return;
    case CONF_REF_CLK:
	long_return= info->ref_clk;
	return (u_char *) & long_return;
    case CONF_LPROBE:
	long_return= info->line_probe;
	return (u_char *) & long_return;
    case CONF_ROW_ST:
	long_return= info->status;
	return (u_char *) & long_return;
    }
    return NULL;
}
	
*/

