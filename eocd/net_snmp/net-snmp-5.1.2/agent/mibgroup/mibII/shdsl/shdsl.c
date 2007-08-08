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

#include "shdsl.h"
#include "struct.h"
#include "util_funcs.h"
#include "../sysORTable.h"
#include "../interfaces.h"

#include "channel_db.h"

//------- Global definitions -------------//
typedef struct{
    char *name[SPAN_NAME_LEN];
    int index;
} shdsl_channel;


/*---- global vars ----*/
struct app_comm *comm;
char driver_dir_path[]="/root/snmp/";


char interface_ind;
char interface_name[256];
int unit_index = 0;
int endp_index = 0;



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

struct variable3 shdsl_endp_conf[] = {
    {ENDP_CONF_PROF, ASN_OCTET_STR, RONLY, var_EndpointConfEntry, 3, {4,1,3}},    
};

/*
struct variable3 shdsl_endp_currstat[] = {
    { ENDP_STAT_CUR_ATN, ASN_INTEGER, RONLY, var_EndpointCurrEntry, 3, {5,1,3} },
    { ENDP_STAT_CUR_SNRMGN, ASN_INTEGER, RONLY, var_EndpointCurrEntry, 3, {5,1,4} },
    { ENDP_STAT_CUR_STATUS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,5} },
    { ENDP_STAT_CUR_ES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,6} },
    { ENDP_STAT_CUR_SES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,7} },
    { ENDP_STAT_CUR_CRC,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,8} },
    { ENDP_STAT_CUR_LOSWS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,9} },
    { ENDP_STAT_CUR_UAS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,10} },
    { ENDP_STAT_CUR_15MEL,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,11} },
    { ENDP_STAT_CUR_15M_ES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,12} },
    { ENDP_STAT_CUR_15M_SES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,13} },
    { ENDP_STAT_CUR_15M_CRC,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,14} },
    { ENDP_STAT_CUR_15M_LOSWS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,15} },
    { ENDP_STAT_CUR_15M_UAS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,16} },
    { ENDP_STAT_CUR_1DEL,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,17} },
    { ENDP_STAT_CUR_1D_ES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,18} },
    { ENDP_STAT_CUR_1D_SES,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,19} },
    { ENDP_STAT_CUR_1D_CRC,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,20} },
    { ENDP_STAT_CUR_1D_LOSWS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,21} },
    { ENDP_STAT_CUR_1D_UAS,ASN_UNSIGNED,RONLY, var_EndpointCurrEntry, 3, {5,1,22} },
};
*/
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

void
init_shdsl(void)
{

    comm = init_comm();
    if(!comm){
	DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
	return;
    }

    /*
     * register ourselves with the agent to handle our mib tree 
     */
    REGISTER_MIB("mibII/hdsl2ShdslSpanConf", shdsl_spanconf, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslSpanStatus", shdsl_spanstat, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslInventory", shdsl_inventory, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslEndpointConf", shdsl_endp_conf, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslEndpointMaint", shdsl_endp_maint, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslUnitMaint", shdsl_unit_maint, variable3,
                 hdsl2Shdsl_variables_oid);

    REGISTER_MIB("mibII/hdsl2shdslSpanConf", shdsl_conf_prof, variable3,
                 hdsl2Shdsl_variables_oid);



//    DEBUGMSGTL(("mibII/hdsl2Shdsl","register variables"));
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
	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
	    return -1;
	}

	fr2 = comm_request(comm,fr1);
	if( !fr2 ){
	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Reqest failed"));
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
    shdsl_channel_elem tbl[SHDSL_MAX_CHANNELS];
    int tbl_size = 0;

    DEBUGMSGTL(("mibII/shdsl", "DSL indexes:\n"));    
    for( i=0;i<ifs_num;i++){
	ifs_ind[i] = Interface_Index_By_Name(ifs_name[i],strlen(ifs_name[i]));
	DEBUGMSGTL(("mibII/shdsl", "%s: index=%d\n",ifs_name[i],ifs_ind[i]));	
	if(min_ind > ifs_ind[i] || (min_ind==-1) ){
	    min_ind = ifs_ind[i];
	    min_i = i;
	}
    }

//    DEBUGMSGTL(("mibII/shdsl", "dslIfIndex, exact = %d, ifs_num=%d\n",exact,ifs_num));
//    dbg_oid("Input OID: ",name,*length);
//    dbg_oid("Local OID: ",vp->name,vp->namelen);
/*    DEBUGMSGTL(("mibII/shdsl", "namelen=%d,length=%d\n",vp->namelen,*length));    

    DEBUGMSGTL(("mibII/shdsl", "input OID print\n"));
    for(i=0;i<*length;i++)
	DEBUGMSGTL(("mibII/shdsl", "name[%d]=%d\n",i,name[i]));
*/    
    
    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));

    if( (base_compare = snmp_oid_compare(name,oid_min,newname,oid_min)) > 0){
	DEBUGMSGTL(("mibII/shdsl", "GETNEXT with greater OID than base OID\n"));  
	return MATCH_FAILED;
    }
    
    if( exact ){
	if( base_compare || (*length < vp->namelen+1) )
	    return MATCH_FAILED;
	// check that name[vp->namelen] is DSL index
	for(i=0;i<ifs_num;i++){
	    if( name[vp->namelen] == ifs_ind[i] ){
		interface_ind = i;
		strcpy(interface_name,ifs_name[i]);
		break;
	    }
	}
	if( i == ifs_num )
	    return MATCH_FAILED;
    } else {
	DEBUGMSGTL(("mibII/shdsl", "Not exact, base_compare=%d\n",base_compare)); 
	// 1. OID for GETNEXT > vp base OID
	// 2. Index of GETNEXT > max index
	if( base_compare < 0 || ( !base_compare && *length<=vp->namelen) ){
	     DEBUGMSGTL(("mibII/shdsl", "base_compare < 0\n",base_compare));
	    memcpy((char *) name, (char *) newname,
    		   ((int) vp->namelen + 1) * sizeof(oid));
	    *length = vp->namelen + 1;
	    if( !ifs_num )
		return MATCH_FAILED;
	    name[vp->namelen] = min_ind;
	    strcpy(interface_name,ifs_name[min_i]);
	    interface_ind  = min_i;
	} else if( !base_compare ){
	    DEBUGMSGTL(("mibII/shdsl", "(base_compare=0), name[vp->namelen]=%d \n",name[vp->namelen]));
	    min_ind = -1;
	    min_i = -1;
	    for( i=0;i<ifs_num;i++){
		DEBUGMSGTL(("mibII/shdsl", "Check index %d\n",ifs_ind[i]));
		if( ifs_ind[i] > name[vp->namelen] ){
		    DEBUGMSGTL(("mibII/shdsl", "ind > input ind\n"));		
		    if( min_ind > ifs_ind[i] || min_ind == -1){
			min_ind = ifs_ind[i];
			min_i = i;
		    }
		}
	    }
	    DEBUGMSGTL(("mibII/shdsl", "Result min_ind = %d, min_i=%d\n",min_ind,min_i));
	    if( (min_ind > 0) && (min_ind > name[vp->namelen]) ){
	    	DEBUGMSGTL(("mibII/shdsl", "Apply new ind min_ind = %d, min_i=%d\n",min_ind,min_i));
	        strcpy(interface_name,ifs_name[min_i]);
		interface_ind = min_i;
		name[vp->namelen] = min_ind;
	    }else{
	    	DEBUGMSGTL(("mibII/shdsl", "FAIL TO MATCH min_ind = %d, min_i=%d\n",min_ind,min_i));	    
		return MATCH_FAILED;
	    }
	} else
	    return MATCH_FAILED;
    }
    *write_method = 0;
    *var_len = sizeof(long); 
    dbg_oid("Result_oid",name,*length);
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
    int iface, ind;
    struct channel_conf cfg;     
    static char ConfProfile[200]="DEFVAL";
    static char ConfAlarmProfile[200]="DEFVAL";
/*
    DEBUGMSGTL(("mibII/shdsl", "var_SpanConfEntry: *lenght=%d namelen=%d \n",*length,vp->namelen));
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\nexact= %d\n", exact));
*/		

    if ( ( iface=header_ifIndex(vp, name, length, exact, var_len,write_method) )
	     == MATCH_FAILED || (*length != vp->namelen+1)){
//	DEBUGMSG(("mibII/shdsl", "MATCH_FAILED\n"));	
        return NULL;
    }
//    DEBUGMSGTL(("mibII/shdsl", "iface= %d\n", iface));











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















    if( eocd_read_conf(interface_name,&eocd,&cfg) ){
//	DEBUGMSG(("mibII/shdsl", "Error retreiving configuration info for if=%s\n",interface_name));	
	return NULL;
    }
//    DEBUGMSG(("mibII/shdsl", "Form response\n"));	
    switch (vp->magic) {
    case CONF_NREPS:
	*var_len=sizeof(int);
	long_return=(cfg.ndevs > 2) ? cfg.ndevs-2 : 0;
	return (u_char *) & long_return;
    case CONF_PRFL:
	sscanf(interface_name,"dsl%d",&ind);
	sprintf(ConfProfile,"sigrand%d",ind);
	*var_len=strlen(ConfProfile);
	return (u_char *)ConfProfile;	
    case CONF_ALARMPRFL:
	*var_len=strlen(ConfAlarmProfile);
	return (u_char *)ConfAlarmProfile;	
    default:
//        DEBUGMSGTL(("snmpd", "unknown sub-id %d in var_interfaces\n",
//                    vp->magic));
	break;
    }
    return NULL;
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
    struct channel_conf cfg;
    
/*
    DEBUGMSGTL(("mibII/shdsl", "var_SpanStatusEntry: \n"));
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\nexact= %d\n", exact));
*/		
    // check conditions 

    if ( ( iface=header_ifIndex(vp, name, length, exact, var_len,write_method) )
		== MATCH_FAILED ){
//	DEBUGMSG(("mibII/shdsl", "MaTH_FAILED\n"));	
        return NULL;
    }
//    DEBUGMSG(("mibII/shdsl", "iface= %d\n", iface));










    if( eocd_read_conf(interface_name,&eocd,&cfg) ){
//	printf("failed\n");
	return NULL;
    }
    


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


















    //---- ack ----//
    switch (vp->magic) {
    case STATUS_NAVAILREPS:
	long_return=(cfg.ndevs > 2) ? cfg.ndevs-2 : 0;    
	return (u_char *) & long_return;
    case STATUS_MAXATTLRATE:
	if( cfg.max_att_lrate ){
	    long_return = (cfg.max_att_lrate + 8) * 1000;
	} else {
	    long_return = 0;
	}
	return (u_char *) & long_return;
    case STATUS_ACTLRATE:
	if( cfg.act_lrate ){
	    long_return = (cfg.act_lrate + 8) * 1000;
	} else {
	    long_return = 0;	
	}
	return (u_char *) & long_return;
    case STATUS_TRNSMSNMODCUR:
	long_return = *((unsigned char*)&cfg.annex);
	*var_len = sizeof(char);
	return (u_char *)&long_return;
    case STATUS_MAXATTPRATE:
	long_return = cfg.max_att_lrate * 1000;
	return (u_char *) & long_return;
    case STATUS_ACTPRATE:
	long_return = cfg.act_lrate * 1000;
	return (u_char *) & long_return;
    default:
	break;
    }
    
    return NULL;
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
    struct channel_conf cfg;
    

//DEBUGMSGTL(("mibII/shdsl", "\n-------------------------------\nheader_InvIndex: \n"));
/*
DEBUGMSGOID(("mibII/shdsl", name, *length));		    
DEBUGMSGTL(("mibII/shdsl", "\nheader_InvIndex: "));
DEBUGMSGOID(("mibII/shdsl", vp->name, vp->namelen));		    
DEBUGMSGTL(("mibII/shdsl", "\n"));



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



















*/    // If OID is belong to Inventory Table
    if( (iface = header_ifIndex(vp, name, length,1/*exact = 1*/, var_len,write_method)) 
	    != MATCH_FAILED ){ 	// OID belongs to Invetory table and ifIdex is valid
// DEBUGMSGTL(("mibII/shdsl", "\nOID belong to Inv Table\n"));
        if( eocd_read_conf(interface_name,&eocd,&cfg) ){
//		DEBUGMSG(("mibII/shdsl", "Error retreiving configuration info for if=%s\n",interface_name));	
		return MATCH_FAILED;
        }
// DEBUGMSGTL(("mibII/shdsl", "Get conf - Ok\n"));
	if( exact ){ // Need exact MATCH
// DEBUGMSGTL(("mibII/shdsl", "Exact Match\n"));
	    if( (*length >= vp->namelen+2) && 
		    (name[vp->namelen+1] > 0) && 
		    (name[vp->namelen+1]<= cfg.ndevs) ){
// DEBUGMSGTL(("mibII/shdsl", "Unit #%d\n",name[vp->namelen+1]));
		return name[vp->namelen+1];
	    } else {
// DEBUGMSGTL(("mibII/shdsl", "Exact MISMATCH\n"));
		return MATCH_FAILED;
	    }
	} else { // Need next regenerator index
// DEBUGMSGTL(("mibII/shdsl", "Nonexact Match\n"));
	    if( *length >= vp->namelen+2 &&
		    (name[vp->namelen+1]+1 > 0) &&
		    (name[vp->namelen+1]+1 <= cfg.ndevs) ){
// DEBUGMSGTL(("mibII/shdsl", "Next unit after %d\n",name[vp->namelen+1]));
		name[vp->namelen+1]++;
		return name[vp->namelen+1];
	    }else if( *length == vp->namelen + 1 ){
		name[vp->namelen+1] = STU_C;
// DEBUGMSGTL(("mibII/shdsl", "First unit \n"));
		*length = vp->namelen+2;
		return name[vp->namelen+1];
	    }
	}
    }
    
// DEBUGMSGTL(("mibII/shdsl", "OID does not belong to Inventory Table OR NO next regenerator\n"));
    // OID does not belong to Inventory Table OR NO next regenerator 
    if( exact ){
	return MATCH_FAILED;
    }
    
    if( (iface=header_ifIndex(vp, name, length, exact, var_len,write_method)) 
		== MATCH_FAILED ){ // No next interface or In OID is lager than Inventory Table OIDs
	return MATCH_FAILED;
    }

// DEBUGMSGTL(("mibII/shdsl", "Next iface = %d \n",iface));
    if( eocd_read_conf(interface_name,&eocd,&cfg) ){ // Error getting configuration about channel
//	DEBUGMSG(("mibII/shdsl", "Error retreiving configuration info for if=%s\n",interface_name));	
	return MATCH_FAILED;
    }
// DEBUGMSGTL(("mibII/shdsl", "Getting config - OK\n"));
    name[vp->namelen+1] = STU_C;
    *length = vp->namelen + 2;
//DEBUGMSGTL(("mibII/shdsl", "Result: unit #%d\n",name[vp->namelen+1]));
    return name[vp->namelen+1];
}


u_char *
var_InventoryEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    shdsl_unit_t Info, *info = &Info;
    int unit;
    size_t length1;
/*
    DEBUGMSGTL(("mibII/shdsl", "var_InventoryEntry: \n"));
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\nexact= %d\n", exact));
*/		
    if ( ( unit = header_unitIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED )
        return NULL;
    
    if( eocd_unit_info(interface_ind,unit,&eocd,info) ){
//	DEBUGMSG(("mibII/shdsl", "Error retreiving UNIT configuration info for if=%s, unit=%d\n",interface_name,unit));	
	return NULL;
    }





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









    
//    DEBUGMSGTL(("mibII/shdsl", " Result unit = %d\n",unit));
    //---- ack ----//
  switch (vp->magic) {
    case INV_VID:
	strcpy(return_buf,info->vID);    
	*var_len = strlen(info->vID);
	return (u_char *)return_buf;
    case INV_VMODELNUM:
	strcpy(return_buf,info->vModelN);    
	*var_len = strlen(info->vModelN);
	return (u_char *)return_buf;
    case INV_VSERNUM:
	strcpy(return_buf,info->vSerN);    
	*var_len = strlen(info->vSerN);
	return (u_char *)return_buf;
    case INV_VEOCSV:
	*var_len = sizeof(info->vEOCSwVer);
	long_return=info->vEOCSwVer;
	return (u_char *)&long_return;
    case INV_STANDARDV:
	long_return=info->StandardVer;
	*var_len = sizeof(info->StandardVer);
	return (u_char *)&long_return;
    case INV_VLISTNUM:
	strcpy(return_buf,info->vListNum);
	*var_len = strlen(info->vListNum);
	return (u_char *)return_buf;
    case INV_VISSUENUM:
	strcpy(return_buf,info->vIssueNum);
	*var_len = strlen(info->vIssueNum);
	return (u_char *)return_buf;
    case INV_EQCODE:
	strcpy(return_buf,info->EqCode);
	*var_len = strlen(info->EqCode);
	return (u_char *)return_buf;
    case INV_VOTHER:
	strcpy(return_buf,info->vOther);    
	*var_len = strlen(info->vOther);
	return (u_char *)return_buf;
    case INV_TRNSMODECPB:
	*var_len = sizeof(info->TransModeCpb);
	long_return=info->TransModeCpb;
	return (u_char *)&long_return;
    }

    return NULL;
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
    struct channel_conf cfg;
    
/*
DEBUGMSGTL(("mibII/shdsl", "\n---------------------------------\nheader_endpIndex: \n"));
DEBUGMSGTL(("mibII/shdsl", "Input OID: "));
DEBUGMSGOID(("mibII/shdsl", name, *length));		    
DEBUGMSG(("mibII/shdsl", "\n"));

DEBUGMSGTL(("mibII/shdsl", "Local OID: "));
DEBUGMSGOID(("mibII/shdsl", vp->name, vp->namelen));		    
DEBUGMSG(("mibII/shdsl", "\n"));

*/
    if ( ( unit = header_unitIndex(vp,name,length, 1 /*exact = 1*/,var_len,write_method) )
	    != MATCH_FAILED ){

	unit_index = unit;



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











	if( exact ){ // Need exact MATCH
//	    DEBUGMSGTL(("mibII/shdsl", "Exact Match\n"));
	    if( (*length >= vp->namelen+3) ){
		switch( unit ){
		case STU_C:
		    if( name[vp->namelen+2] != NETWORK_SIDE )
			return MATCH_FAILED;
		    else
			return name[vp->namelen+2];
		    break;
		case STU_R:
		    if( name[vp->namelen+2] != CUSTOMER_SIDE )		
			return MATCH_FAILED;
		    else
			return name[vp->namelen+2];
		    break;
		case SRU1:
		case SRU2:
		case SRU3:
		case SRU4:
		case SRU5:
		case SRU6:
		case SRU7:
		case SRU8:
		    if( name[vp->namelen+2] == CUSTOMER_SIDE || 
			name[vp->namelen+2] == NETWORK_SIDE )
			return name[vp->namelen+2];
		    else
			return MATCH_FAILED;
		default:
		    return MATCH_FAILED;
		}
	    } else
		return MATCH_FAILED;
	} else { // Nonexact match
	    if( unit >= SRU1 && unit <=SRU8 ){
		if( *length >= vp->namelen+3 ){
		    if( name[vp->namelen+2] == NETWORK_SIDE ){
			name[vp->namelen+2] = CUSTOMER_SIDE;
			return name[vp->namelen+2];
		    }
		}else if( *length == vp->namelen+2 ){
		    name[vp->namelen+2] == NETWORK_SIDE;
		    *length = vp->namelen+3;
    		    return name[vp->namelen+2];	
		}
	    }
	}
    }
	
//    DEBUGMSGTL(("mibII/shdsl", "OID does not belong to Inventory Table OR NO next regenerator\n"));
    // OID does not belong to Inventory Table OR NO next regenerator 
    if( exact ){
	return MATCH_FAILED;
    }
    
    if( (unit=header_unitIndex(vp, name, length, exact, var_len,write_method)) 
	    == MATCH_FAILED ){ // No next interface or In OID is lager than Inventory Table OIDs
	return MATCH_FAILED;
    }
//    DEBUGMSGTL(("mibII/shdsl", "Next unit = %d \n",unit));

    switch( unit ){
    case STU_C:
	name[vp->namelen+2] = NETWORK_SIDE;
	break;
    case STU_R:
	name[vp->namelen+2] = CUSTOMER_SIDE;    
	break;
    case SRU1:
    case SRU2:
    case SRU3:
    case SRU4:
    case SRU5:
    case SRU6:
    case SRU7:
    case SRU8:
	name[vp->namelen+2] = NETWORK_SIDE;
	break;
    default:
	return MATCH_FAILED;
    }
    *length = vp->namelen + 3;
//    DEBUGMSGTL(("mibII/shdsl", "Result: unit #%d\n",name[vp->namelen+1]));
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
    struct channel_conf cfg;
    
/*
DEBUGMSGTL(("mibII/shdsl", "\n---------------------------------\nheader_wirePairIndex:\n"));
DEBUGMSGTL(("mibII/shdsl", "Input OID: "));
DEBUGMSGOID(("mibII/shdsl", name, *length));		    
DEBUGMSG(("mibII/shdsl", "\n"));

DEBUGMSGTL(("mibII/shdsl", "Local OID: "));
DEBUGMSGOID(("mibII/shdsl", vp->name, vp->namelen));		    
DEBUGMSG(("mibII/shdsl", "\n"));

*/
    if ( ( endp = header_endpIndex(vp,name,length, 1 /*exact = 1*/,var_len,write_method) )
	    != MATCH_FAILED ){

	endp_index = endp;

	if( exact ){ // Need exact MATCH
//	    DEBUGMSGTL(("mibII/shdsl", "Exact Match\n"));
	    if( (*length >= vp->namelen+4) && (name[vp->namelen+3] == 1) ){
		return name[vp->namelen+3];
	    } else
		return MATCH_FAILED;
	} else { // Nonexact match
	    // Because we hawe only one wire pair - we change it only 
	    // if field is empty
	    if( *length == vp->namelen+3 ){
	        name[vp->namelen+3] = WIRE_PAIR_1;
	        *length = vp->namelen+4;
	    }
	}
    }
	
//    DEBUGMSGTL(("mibII/shdsl", "OID does not belong to Inventory Table OR NO next regenerator\n"));
    // OID does not belong to Inventory Table OR NO next regenerator 






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









    if( exact ){
	return MATCH_FAILED;
    }
    
    if( (endp = header_endpIndex(vp, name, length, exact, var_len,write_method)) 
	    == MATCH_FAILED ){ // No next interface or In OID is lager than Inventory Table OIDs
	return MATCH_FAILED;
    }
//    DEBUGMSGTL(("mibII/shdsl", "Next unit = %d \n",endp));

    name[ vp->namelen+3] = WIRE_PAIR_1;
    *length = vp->namelen + 4;
//    DEBUGMSGTL(("mibII/shdsl", "Result: PAIR #%d\n",name[vp->namelen+3]));
    return name[vp->namelen+2];
}

/*
 * Segment Endpoint Configuration Group
 */
u_char *
var_EndpointConfEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    shdsl_unit_t Info, *info = &Info;
    int pair;
/*
    DEBUGMSGTL(("mibII/shdsl", "\n-----------------------START----------------------------\n"
			"var_EndpointConfEntry. exact= %d\n",exact));
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\n"));    
*/    
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
 * Segment Endpoint Current Status/Performance Group
 */
/*
u_char *
var_EndpointConfEntry(struct variable * vp,
               oid * name,
               size_t * length,
               int exact, size_t * var_len, WriteMethod ** write_method)
{
    shdsl_unit_t Info, *info = &Info;
    int pair;
    DEBUGMSGTL(("mibII/shdsl", "\n-----------------------START----------------------------\n"
			"var_EndpointConfEntry. exact= %d\n",exact));
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\n"));    
    
    if ( ( pair = header_wirePairIndex(vp,name,length,exact,var_len,write_method) )
	    == MATCH_FAILED )
        return NULL;
    
    DEBUGMSGTL(("mibII/shdsl", "Result pair = %d\n-------------------END------------------------\n",pair));
    //---- ack ----//
    switch (vp->magic) {
    case ENDP_STAT_CUR_ATN:
	break;
    case ENDP_STAT_CUR_SNRMGN:
	break;
    case ENDP_STAT_CUR_STATUS:
	break;
    case ENDP_STAT_CUR_ES:
	break;
    case ENDP_STAT_CUR_SES:
	break;
    case ENDP_STAT_CUR_CRC:
	break;
    case ENDP_STAT_CUR_LOSWS:
	break;
    case ENDP_STAT_CUR_UAS:
	break;
    case ENDP_STAT_CUR_15MEL:
	break;
    case ENDP_STAT_CUR_15M_ES:
	break;
    case ENDP_STAT_CUR_15M_SES:
	break;
    case ENDP_STAT_CUR_15M_CRC:
	break;
    case ENDP_STAT_CUR_15M_LOSWS:
	break;
    case ENDP_STAT_CUR_15M_UAS:
	break;
    case ENDP_STAT_CUR_1DEL:
	break;
    case ENDP_STAT_CUR_1D_ES:
	break;
    case ENDP_STAT_CUR_1D_SES:
	break;
    case ENDP_STAT_CUR_1D_CRC:
	break;
    case ENDP_STAT_CUR_1D_LOSWS:
	break;
    case ENDP_STAT_CUR_1D_UAS:
	break;
    default:
	break;
    }
    
    return NULL;
}



/*
 *
 * ---------------------- Maintenance Group  ------------------------
 *
 */

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
*/    
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
/*
    DEBUGMSGTL(("mibII/shdsl", "\n-----------------------START----------------------------\n"
			"var_UnitMaintEntry. exact= %d\n",exact));
    DEBUGMSGOID(("mibII/shdsl", name, *length));
    DEBUGMSG(("mibII/shdsl", "\n"));    
*/    
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

*/    

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
*/		min_i = i;
	    } else {
/*		DEBUGMSGTL(("mibII/shdsl", "Compare newname with tmpname\n"));	    
		DEBUGMSGTL(("mibII/shdsl", "tmpname OID: "));
		DEBUGMSGOID(("mibII/shdsl", tmpname, tmplen));		    
		DEBUGMSG(("mibII/shdsl", "\n"));
*/	    	result = snmp_oid_compare(newname,newlen+sublen,tmpname,tmplen);
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
*/
    
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
	
