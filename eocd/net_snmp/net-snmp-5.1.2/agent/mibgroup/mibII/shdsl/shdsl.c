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

//--------- DEBUG OUTPUT -----------------//


//#define DEBUG_ON
#define DERR 1
#define DINFO 5
#define DALL 10
#define PDEBUG(flag,fmt,args...)
#ifdef DEBUG_ON
#       undef PDEBUG
#       define PDEBUG(flag,fmt,args...) \
		if( flag ) \
			printf("%s: " fmt " \n",__FUNCTION__, ## args  )
#endif


//------- Global definitions -------------//
#define CACHE_INT 0
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

/* typedef struct{ */
/*     struct timeval tv; */
/*     span_status_payload p; */
/* } shdsl_spanstatus_elem; */



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

struct variable3 shdsl_endp_1daystat[] = {
{ ENDP_1D_INT,ASN_UNSIGNED,NOACCESS,var_1DayIntervalEntry,3,{7,1,1} },
{ ENDP_1D_MONSECS,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{7,1,2} },
{ ENDP_1D_ES,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{7,1,3} },
{ ENDP_1D_SES,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{7,1,4} },
{ ENDP_1D_CRC,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{7,1,5} },
{ ENDP_1D_LOSWS,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{7,1,6} },
{ ENDP_1D_UAS,ASN_UNSIGNED,RONLY,var_1DayIntervalEntry,3,{7,1,7} },
};

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
*/

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
    PDEBUG(0,"Register Span Conf");
    REGISTER_MIB("mibII/hdsl2ShdslSpanConf", shdsl_spanconf, variable3,
                 hdsl2Shdsl_variables_oid);
				 
    PDEBUG(0,"Register Span Status");
    REGISTER_MIB("mibII/hdsl2shdslSpanStatus", shdsl_spanstat, variable3,
                 hdsl2Shdsl_variables_oid);

	PDEBUG(0,"Register Inventory");
    REGISTER_MIB("mibII/hdsl2shdslInventory", shdsl_inventory, variable3,
                 hdsl2Shdsl_variables_oid);

	/*
	  REGISTER_MIB("mibII/hdsl2shdslEndpointConf", shdsl_endp_conf, variable3,
	  hdsl2Shdsl_variables_oid);
	*/

	REGISTER_MIB("mibII/hdsl2shdslEndpointCurr",shdsl_endp_currstat, variable3,
				 hdsl2Shdsl_variables_oid);


	REGISTER_MIB("mibII/hdsl2shdslEndpoint15min",shdsl_endp_15minstat, variable3,
				 hdsl2Shdsl_variables_oid);

	REGISTER_MIB("mibII/hdsl2shdslEndpoint1day",shdsl_endp_1daystat, variable3,
				 hdsl2Shdsl_variables_oid);

	/*
	  REGISTER_MIB("mibII/hdsl2shdslEndpointMaint", shdsl_endp_maint, variable3,
	  hdsl2Shdsl_variables_oid);

	  REGISTER_MIB("mibII/hdsl2shdslUnitMaint", shdsl_unit_maint, variable3,
	  hdsl2Shdsl_variables_oid);
	*/
		
	REGISTER_MIB("mibII/hdsl2shdslSpanConfProfile", shdsl_conf_prof, variable3,
				 hdsl2Shdsl_variables_oid);
	  
    DEBUGMSGTL(("mibII/hdsl2Shdsl","register variables"));
	PDEBUG(0,"Register finished");
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

	PDEBUG(0,"----------------------- >");
	// ---- DEBUG ----//
   	struct timeval tv1,tv2;
    gettimeofday(&tv1,NULL);
	// ---- DEBUG ----//
	
    if( ((tvcur.tv_sec - tbl_tv.tv_sec) > CACHE_INT) || tverr ){

        ifname[0] = 0;
		tbl_size = 0;
		p = (span_name_payload*)
			comm_alloc_request(APP_SPAN_NAME,APP_GET,ifname,&fr1);

		if( !p ){
    	    DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
    	    return -1;
		}
    
        do{
			set_chan_name(fr1,ifname);
			fr2 = comm_request(comm,fr1);

			if( !fr2 ){
				PDEBUG(0,"mibII/hdsl2Shdsl Reqest failed");
				comm_frame_free(fr1);
				return min_i;
			}
			p = (span_name_payload*)comm_frame_payload(fr2);
			for(i=0;i<p->filled;i++){
				len = strnlen(p->spans[i].name,SPAN_NAME_LEN);
				if( (p->spans[i].t == slave) || 
					(index = ifname_to_index(p->spans[i].name,len)) < 0 )
					continue;

				PDEBUG(0,"save name = %s",p->spans[i].name);
				strncpy(tbl[tbl_size].name,p->spans[i].name,len);
				tbl[tbl_size].index = index;
				if( (tbl_size==0) || tbl[min_i].index > tbl[tbl_size].index  ){
					min_i = tbl_size;
				}
				tbl[tbl_size].units = -1;
				tbl[tbl_size].wires = -1;
				tbl[tbl_size].tv.tv_sec = 0;
				tbl_size++;
			}
			if( !p->last_msg && p->filled){
				strncpy(ifname,p->spans[p->filled-1].name,SPAN_NAME_LEN);
			}
			comm_frame_free(fr2);
		}while( tbl_size<SHDSL_MAX_CHANNELS && !p->last_msg );
		
		comm_frame_free(fr1);
		tbl_tv = tvcur;
    }
			
	//--- DEBUG ---//
    gettimeofday(&tv2,NULL);
	PDEBUG(0,"<------- %d ------------",tv2.tv_usec-tv1.tv_usec);
	//--- DEBUG ---//

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
	// ---- DEBUG ----//
   	struct timeval tv1,tv2;
    gettimeofday(&tv1,NULL);
	// ---- DEBUG ----//

	PDEBUG(0,"---------------------->");
    if( (min_i = chann_names()) < 0 ){
		PDEBUG(0,"Cannot get table of controlling interfaces");
		return MATCH_FAILED;
    }
	PDEBUG(0,"chan names-success. min_i=%d\n",min_i);

    if( !tbl_size ){
		PDEBUG(0,"MATCH FAILED: tbl_size is zero");
		return MATCH_FAILED;
    }

    memcpy((char *) newname, (char *) vp->name,
           (int) vp->namelen * sizeof(oid));

    if( (base_compare = snmp_oid_compare(name,oid_min,newname,oid_min)) > 0){
		return MATCH_FAILED;
    }
    
    if( exact ){
		if( base_compare || (*length < vp->namelen+1) ){
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
		// 1. OID for GETNEXT > vp base OID
		// 2. Index of GETNEXT > max index
		if( base_compare < 0 || ( !base_compare && *length<=vp->namelen) ){
			memcpy((char*)name,(char*)newname,((int)vp->namelen+1)*sizeof(oid));
			*length = vp->namelen + 1;
			name[vp->namelen] = tbl[min_i].index;
			interface_ind  = min_i;
		}else if( !base_compare ){
			min_i = 0;
			min_ind = -1;
			for( i=0;i<tbl_size;i++){
				if( tbl[i].index > name[vp->namelen] ){
					if( (min_ind < 0) || (min_ind > tbl[i].index) ){
						min_i = i;
						min_ind = tbl[i].index;
					}
				}
			}
			if( min_ind>0 ){
				interface_ind = min_i;
				name[vp->namelen] = tbl[min_i].index;
			}else{
				return MATCH_FAILED;
			}
		} else
			return MATCH_FAILED;
    }
	
    *write_method = 0;
    *var_len = sizeof(long); 
    dbg_oid("Result_oid ",name,*length);

	//--- DEBUG ---//
    gettimeofday(&tv2,NULL);
	PDEBUG(0,"<------- %d ------------",tv2.tv_usec-tv1.tv_usec);
	//--- DEBUG ---//
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

	PDEBUG(0,"--------------------->");
    comm = init_comm();
    if(!comm){
        DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
        return NULL;
    }

    if ( ( iface=header_ifIndex(vp, name, length, exact, var_len,write_method) )
         == MATCH_FAILED || (*length != vp->namelen+1)){
		//	DEBUGMSG(("mibII/shdsl", "SpanConf: MATCH_FAILED\n"));	
		//	printf("mibII/shdsl SpanConf: MATCH_FAILED\n");	
		goto exit;
    }

    if( gettimeofday(&tvcur,NULL) ){
		PDEBUG(0,"tverr occured");
		tverr = 1;
    }

    if( !spanconf_tbl[interface_ind] || tverr ||
		(tvcur.tv_sec - spanconf_tbl[interface_ind]->tv.tv_sec > CACHE_INT) ){

        p = (span_conf_payload*)comm_alloc_request(APP_SPAN_CONF,APP_GET,tbl[interface_ind].name,&fr1);
		if( !p ){
			//DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
			PDEBUG(0,"mibII/shdsl Cannot allocate application frame");
    	    goto exit;
		}

		fr2 = comm_request(comm,fr1);
        if( !fr2 ){
			PDEBUG(0,"Error requesting");
    	    goto exit;
		}
		p = (span_conf_payload*)comm_frame_payload(fr2);

        // Cache data
		if( !spanconf_tbl[interface_ind] )
			spanconf_tbl[interface_ind] = malloc(sizeof(shdsl_spanconf_elem));
		spanconf_tbl[interface_ind]->p = *p;
		spanconf_tbl[interface_ind]->tv = tvcur;
    }else{
		p = &spanconf_tbl[interface_ind]->p;
    }

    switch (vp->magic) {
    case CONF_NREPS:
		*var_len=sizeof(int);
		long_return=p->nreps;
		//	long_return= 2;
		return_ptr = (u_char *) & long_return;
		break;
    case CONF_PRFL:
		strncpy(ConfProfile,p->conf_prof,SNMP_ADMIN_LEN);
		//	strncpy(ConfProfile,"profile",SNMP_ADMIN_LEN);
		*var_len=strnlen(ConfProfile,SNMP_ADMIN_LEN);
		return_ptr = (u_char *)ConfProfile;	
		break;
    case CONF_ALARMPRFL:
		goto exit;
		//!!	*var_len=strlen(ConfAlarmProfile);
		//!!	return (u_char *)ConfAlarmProfile;	
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

	PDEBUG(0,"< -------------------");
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

	PDEBUG(0,"--------------------- >");
    comm = init_comm();
    if(!comm){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
		return NULL;
    }

    if ( ( iface=header_ifIndex(vp, name, length, exact, var_len,write_method) )
	     == MATCH_FAILED || (*length != vp->namelen+1)){
		PDEBUG(0,"mibII/shdsl SpanConf: MATCH_FAILED");	
        goto exit;
    }

    if( gettimeofday(&tvcur,NULL) ){
		PDEBUG(0,"tverr occured");
		tverr = 1;
    }

    if( !spanstatus_tbl[interface_ind] || tverr ||
	    ((tvcur.tv_sec - spanstatus_tbl[interface_ind]->tv.tv_sec)>CACHE_INT) ){
        p = (span_status_payload*)comm_alloc_request(APP_SPAN_STATUS,APP_GET,tbl[interface_ind].name,&fr1);
		if( !p ){
			//DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
			PDEBUG(0,"Cannot allocate application frame");
			goto exit;
		}
		fr2 = comm_request(comm,fr1);
		if( !fr2 ){
			PDEBUG(0,"Error requesting");
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
		*/
	case STATUS_MAXATTPRATE:
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
	PDEBUG(0,"< -------------------");
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
    struct app_frame *fr1=NULL,*fr2=NULL;
    span_params_payload *p;
    int ret_val = MATCH_FAILED;
    // Cacheing
    struct timeval tvcur;
    int tverr = 0;
	// ---- DEBUG ----//
   	struct timeval tv1,tv2;
    gettimeofday(&tv1,NULL);
	PDEBUG(0,"--------------------- >");
	// ---- DEBUG ----//

	// If OID is belong to Inventory Table
    if( (iface = header_ifIndex(vp, name, length,1/*exact = 1*/, var_len,write_method)) 
	    != MATCH_FAILED ){ 	// OID belongs to Invetory table and ifIdex is valid
		PDEBUG(0,"OID belong to Inv Table, interface_ind=%d,if = %s\n",interface_ind,
				tbl[interface_ind].name);
        
		if( gettimeofday(&tvcur,NULL) ){
			PDEBUG(0,"tverr occured");
			tverr = 1;
		}

		if( ((tvcur.tv_sec - tbl[interface_ind].tv.tv_sec) > CACHE_INT) || tverr || 
			tbl[interface_ind].units < 0 ){
			p = (span_params_payload*)comm_alloc_request(APP_SPAN_PARAMS,
														 APP_GET,tbl[interface_ind].name,&fr1);
			PDEBUG(0,"Refresh span params");

			if( !p ){
				PDEBUG(0,"ERROR: Cannot allocate application frame");
				goto exit;
			}
			fr2 = comm_request(comm,fr1);
			if( !fr2 ){
				PDEBUG(0,"ERROR: Error requesting\n");
				goto exit;
			}
			p = (span_params_payload*)comm_frame_payload(fr2);
			// Cache data
			tbl[interface_ind].tv = tvcur;
			tbl[interface_ind].units = p->units;
			tbl[interface_ind].wires = p->loops;

			PDEBUG(0,"NEW %s params: units=%d, loop=%d",tbl[interface_ind].name,p->units,p->loops);
		}
		PDEBUG(0,"Get params info for %s: units(%d) loops(%d)",tbl[interface_ind].name,tbl[interface_ind].units,
				tbl[interface_ind].wires);

		if( exact ){ // Need exact MATCH
			//	    printf("unitIndex: Exact match\n");
			if( (*length >= vp->namelen+2) && 
				(name[vp->namelen+1] > 0) && 
				(name[vp->namelen+1]<= tbl[interface_ind].units) ){
				//		    printf("unitIndex: Exact match - OK\n");
				ret_val = name[vp->namelen+1];
				goto exit;
			} else {
				//		printf("unitIndex: Exact match - FAIL\n");
				goto exit;
			}
		} else { // Need next regenerator index
			if( *length >= vp->namelen+2 &&
				(name[vp->namelen+1]+1 > 0) &&
				(name[vp->namelen+1]+1 <= tbl[interface_ind].units) ){
				//		printf("unitIndex: Next unit after %d\n",name[vp->namelen+1]);
				name[vp->namelen+1]++;
				ret_val = name[vp->namelen+1];
				goto exit;
			}else if( *length == vp->namelen + 1 ){
				name[vp->namelen+1] = stu_c;
				//		printf("unitIndex:  First unit \n");
				*length = vp->namelen+2;
				ret_val = name[vp->namelen+1];
				goto exit;
			}
		}
    }
    
    if( exact ){
		PDEBUG(0,"Exact match - seems failed");
		goto exit;
    }
    
    if( (iface=header_ifIndex(vp, name, length, exact, var_len,write_method)) 
		== MATCH_FAILED ){ // No next interface or In OID is lager than Inventory Table OIDs
		PDEBUG(0,"Search for next iface failed");
		goto exit;
    }
    
    if( fr2 ){
		comm_frame_free(fr2);
		fr2 = NULL;
    }

    if( ((tvcur.tv_sec - tbl[interface_ind].tv.tv_sec) > CACHE_INT) || tverr || 
		tbl[interface_ind].units < 0 ){
	
		if( !fr1 ){	
			p = (span_params_payload*)comm_alloc_request(APP_SPAN_PARAMS,
														 APP_GET,tbl[interface_ind].name,&fr1);
		}else{
			p = (span_params_payload*)comm_frame_payload(fr1);
		}
		    
		if( !p ){
			PDEBUG(0,"Cannot allocate application frame");
    	    goto exit;
		}
		fr2 = comm_request(comm,fr1);
		if( !fr2 ){
			PDEBUG(0,"Error requesting");
    	    goto exit;
		}
		p = (span_params_payload*)comm_frame_payload(fr2);

		PDEBUG(0,"Get params info for %s: units(%d) loops(%d)\n",tbl[interface_ind].name,p->units,p->loops);    

		tbl[interface_ind].units = p->units;
		tbl[interface_ind].wires = p->loops;
		tbl[interface_ind].tv = tvcur;
    }
	
    name[vp->namelen+1] = stu_c;
    *length = vp->namelen + 2;
	PDEBUG(0,"Result: unit #%d\n",name[vp->namelen+1]);
    ret_val = name[vp->namelen+1];
 exit:
    if( fr1 )
		comm_frame_free(fr1);
    if( fr2 )
		comm_frame_free(fr2);

	//--- DEBUG ---//
    gettimeofday(&tv2,NULL);
	PDEBUG(0,"<------- %d ------------",tv2.tv_usec-tv1.tv_usec);
	//--- DEBUG ---//
    return ret_val;
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


	PDEBUG(0,"------------------------>");
    comm = init_comm();
    if(!comm){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
		return NULL;
    }

    if ( ( unit = header_unitIndex(vp,name,length,exact,var_len,write_method) )
		 == MATCH_FAILED ){
		PDEBUG(0,"Cannot find unit");
		goto exit;
    }

	// ------------- Need cacheing ------------------//
	p = (inventory_payload*)comm_alloc_request(APP_INVENTORY,APP_GET,
											   tbl[interface_ind].name,&fr1);
	if( !p ){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
		PDEBUG(0,"Cannot allocate application frame");
		goto exit;
	}

	p->unit = unit;
	fr2 = comm_request(comm,fr1);
	if( !fr2 && exact ){
		PDEBUG(0,"Error requesting");
		goto exit;
	}
    
	while( !fr2 ){
		
		if ( ( unit = header_unitIndex(vp,name,length,exact,var_len,write_method) )
			 == MATCH_FAILED ){
			PDEBUG(0,"Cannot find unit");
			goto exit;
		}

		p = (inventory_payload*)comm_alloc_request(APP_INVENTORY,APP_GET,
												   tbl[interface_ind].name,&fr1);
		if( !p ){
			DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
			PDEBUG(0,"Cannot allocate application frame");
			goto exit;
		}
		p->unit = unit;
		if( fr2 )
			comm_frame_free(fr2);
		fr2 = comm_request(comm,fr1);
	}
    
	if( !fr2 ){
		PDEBUG(0,"Error requesting");
		goto exit;
	}
	p = (inventory_payload*)comm_frame_payload(fr2);
	//~~~~~~~~~~~~~~~~~~ Need cacheing ~~~~~~~~~~~~~~~~~~~~~~~~`//

	resp = &p->inv;

    //---- ack ----//
    switch (vp->magic) {
    case INV_VID:
		*var_len = sizeof(resp->ven_id);
		memset(return_buf,0,*var_len);
		strncpy(return_buf,resp->ven_id,*var_len);    
		/* 		strncpy(return_buf,"111",10); */
		/* 		*var_len = strlen(return_buf); */
		return_ptr = (u_char *)return_buf;
		break;
    case INV_VMODELNUM:
		*var_len = sizeof(resp->ven_model);
		memset(return_buf,0,*var_len);
		strncpy(return_buf,resp->ven_model,*var_len);    
		/* 		strncpy(return_buf,"222",10); */
		/* 		*var_len = strlen(return_buf); */
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
		*/
	}

 exit:
    if( fr1 )
		comm_frame_free(fr1);
    if( fr2 )
		comm_frame_free(fr2);
    comm_free(comm);
    comm = NULL;
	PDEBUG(0,"<------------------------");
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

	// ---- DEBUG ----//
   	struct timeval tv1,tv2;
    gettimeofday(&tv1,NULL);
	PDEBUG(0,"--------------------- >");
	// ---- DEBUG ----//

    if ( ( unit = header_unitIndex(vp,name,length, 1 /*exact = 1*/,var_len,write_method) )
		 != MATCH_FAILED ){
		unit_index = unit;
	    PDEBUG(0,"Result : if(%s) unit(%d)",tbl[interface_ind].name,unit_index);

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

			PDEBUG(0,"nonexact match: unit(%d) side(%d)\n",unit,name[vp->namelen+2]);
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
	PDEBUG(0,"NEXT Result : if(%s) unit(%d)",tbl[interface_ind].name,unit_index);

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

	//--- DEBUG ---//
    gettimeofday(&tv2,NULL);
	PDEBUG(0,"<------- %d ------------",tv2.tv_usec-tv1.tv_usec);
	//--- DEBUG ---//
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
    struct app_frame *fr1=NULL,*fr2=NULL;
    span_params_payload *p;
    int ret_val = MATCH_FAILED;
    // Cacheing
    struct timeval tvcur;
    int tverr = 0;

	// ---- DEBUG ----//
   	struct timeval tv1,tv2;
    gettimeofday(&tv1,NULL);
	PDEBUG(0,"--------------------- >");
	// ---- DEBUG ----//

    if ( ( endp = header_endpIndex(vp,name,length, 1 /*exact = 1*/,var_len,write_method) )
		 != MATCH_FAILED ){
		endp_index = endp;
		int k = tbl[interface_ind].wires;
		PDEBUG(0,"Result : if(%s) unit(%d) endp(%d)",tbl[interface_ind].name,unit_index,endp_index);

		if( exact ){ // Need exact MATCH
			PDEBUG(0,"Exact Match");
			if( (*length >= vp->namelen+4) && 
				(name[vp->namelen+3] > 0) && (name[vp->namelen+3] <= tbl[interface_ind].wires) ){
				ret_val =  name[vp->namelen+3];
				goto exit;
			} else {
				goto exit;
			}
		} else { // Nonexact match
			// Because we have only one wire pair - we change it only 
			// if field is empty
			if( *length == vp->namelen+3 ){
				name[vp->namelen+3] = 1; // first pair
				*length = vp->namelen+4;
				ret_val = name[vp->namelen+3];
				goto exit;
			}else{
				if( name[vp->namelen+3] <= 0 )
					goto exit;
				if( name[vp->namelen+3]+1 <= tbl[interface_ind].wires ){
					*length = vp->namelen+4;
					ret_val = (++name[vp->namelen+3]);
					goto exit;
				}
			}
		}
    }
	
	PDEBUG(0,"Exact match failed");
    if( exact ){
		goto exit;
    }
    
    if( (endp = header_endpIndex(vp, name, length, exact, var_len,write_method)) 
	    == MATCH_FAILED ){ // No next interface or In OID is lager than Inventory Table OIDs
		goto exit;
    }
    endp_index = endp;
	PDEBUG(0,"NEXT Result : if(%s) unit(%d) endp(%d)",tbl[interface_ind].name,unit_index,endp_index);
    name[ vp->namelen+3] = 1; // First pair
    *length = vp->namelen + 4;
    ret_val = name[vp->namelen+3];

 exit:
    if( fr1 )
		comm_frame_free(fr1);
    if( fr2 )
		comm_frame_free(fr2);

	//--- DEBUG ---//
    gettimeofday(&tv2,NULL);
	PDEBUG(0,"<------- %d ------------",tv2.tv_usec-tv1.tv_usec);
	//--- DEBUG ---//
    return ret_val;
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

    PDEBUG(0,"-----!!!!!!!!!!!!!!-------->");

	// ------------------------------ Obtain requested information ------------------------------- //

    if ( ( pair = header_wirePairIndex(vp,name,length,exact,var_len,write_method) )
		 == MATCH_FAILED )
        goto exit;
       
    PDEBUG(0,"Result : if(%s) unit(%d) side(%d) pair(%d)",tbl[interface_ind].name,unit_index,endp_index,pair);

    p = (endp_cur_payload*)comm_alloc_request(APP_ENDP_CUR,APP_GET,
											  tbl[interface_ind].name,&fr1);
    if( !p ){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
		PDEBUG(0,"var_InventoryEntry: Cannot allocate application frame");
		goto exit;
    }
    p->unit = unit_index;
    p->side = endp_index-1;
    p->loop = pair-1;
    fr2 = comm_request(comm,fr1);
    PDEBUG(0,"Request if(%s) unit(%d) side(%d) loop(%d)",
		   tbl[interface_ind].name,p->unit,p->side,p->loop);
    
    if( !fr2 && exact ){
		PDEBUG(0,"var_InventoryEntry: error requesting");
		goto exit;
    }
    
    while( !fr2 ){
		if ( ( pair = header_wirePairIndex(vp,name,length,exact,var_len,write_method) )
			 == MATCH_FAILED )
    	    goto exit;
    
		PDEBUG(0,"Result (rep): if(%s) unit(%d) side(%d) pair(%d)",tbl[interface_ind].name,unit_index,endp_index,pair);
		set_chan_name(fr1,tbl[interface_ind].name);
		p->unit = unit_index;
		p->side = endp_index-1;
		p->loop = pair-1;
		fr2 = comm_request(comm,fr1);

		PDEBUG(0,"Request (rep): if(%s) unit(%d) side(%d) loop(%d)",
			   tbl[interface_ind].name,p->unit,p->side,p->loop);

    }

    if( !fr2 ){
		PDEBUG(0,"error requesting");
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
    PDEBUG(0,"<------------!!!!!!!!!!!!!1----------\n\n");
    return return_ptr;
}

// ----------------- Perfomance/Status intervals ------------------//

static int 
header_intervalIndex(struct variable *vp,
					 oid * name,
					 size_t * length,
					 int exact, size_t * var_len, WriteMethod ** write_method,app_ids msg_id)
{
    int wire = 0;
    *write_method = 0;
    *var_len = sizeof(long);    // default to 'long' results //
    struct app_frame *fr1=NULL,*fr2=NULL;
    endp_int_payload *p;
    int int_num,ret_val = MATCH_FAILED;
    
    
    PDEBUG(0,"(Input) if(%d) unit(%d) side(%d) pair(%d) int(%d) id(%d)",
		   __FUNCTION__,name[vp->namelen],name[vp->namelen+1],name[vp->namelen+2],
		   name[vp->namelen+3],name[vp->namelen+4],msg_id);

    //-------------- Try requested wire_pair ----------------//
    if ( ( wire = header_wirePairIndex(vp,name,length, 1 /*exact = 1*/,var_len,write_method) )
		 != MATCH_FAILED ){
    
		wire_index = wire;
		if( exact ){
			if( *length != vp->namelen+5 )
				goto exit;
			int_num = name[vp->namelen+4];
		} else {
			if( *length < vp->namelen+4 ){
				return MATCH_FAILED;
			} else if( *length == vp->namelen+3 ){
				int_num = 1;
			}else{
				int_num = name[vp->namelen+4]+1;
			}
		}

		if( exact ){
			p = (endp_int_payload*)comm_alloc_request(msg_id,APP_GET,
													  tbl[interface_ind].name,&fr1);
		}else{
			p = (endp_int_payload*)comm_alloc_request(msg_id,APP_GET_NEXT,
													  tbl[interface_ind].name,&fr1);
		}	
		if( !p ){
			DEBUGMSGTL(("mibII/hdsl2Shdsl","interval15Index: Cannot allocate application frame"));
			PDEBUG(0,"Cannot allocate application frame");
			return MATCH_FAILED;
		}
	    
		p->unit = unit_index;
		p->side = endp_index-1;
		p->loop = wire_index-1;
		p->int_num = int_num;

		PDEBUG(0,"(Output) unit(%d) side(%d) pair(%d) int(%d)",
			   p->unit,p->side,p->loop,p->int_num);
	
		fr2 = comm_request(comm,fr1);
		if( fr2 ){
			p = (endp_int_payload*)comm_frame_payload(fr2);
			//	printf("wirePair: Get params info for %s: units(%d) loops(%d)\n",tbl[interface_ind].name,p->units,p->loops);
			perf_int = p->cntrs;
			name[vp->namelen+4] = p->int_num;
			*length = vp->namelen+5;
			ret_val = name[vp->namelen+4];
			goto exit;
		}
    }
    
    if( exact ){
		goto exit;
    }
    
    //-------------- Switch to next wire_pair ----------------//
    if ( ( wire = header_wirePairIndex(vp,name,length, exact,var_len,write_method) )
		 == MATCH_FAILED ){
		goto exit;
    }
    wire_index = wire;
    if( !fr1 ){
		p = (endp_int_payload*)comm_alloc_request(msg_id,APP_GET_NEXT,
												  tbl[interface_ind].name,&fr1);
    }else{
		p = (endp_int_payload*)comm_frame_payload(fr1);
    }
    
    if( !p ){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","interval15Index: Cannot allocate application frame"));
		PDEBUG(0,"interval15Index: Cannot allocate application frame");
		goto exit;
    }

    p->unit = unit_index;
    p->side = endp_index-1;
    p->loop = wire_index-1;
    p->int_num = 1;

    PDEBUG(0,"Output: unit(%d) side(%d) pair(%d) int(%d)",
		   p->unit,p->side,p->loop,p->int_num);

	if( fr2 )
		comm_frame_free(fr2);
    fr2 = comm_request(comm,fr1);
    if( !fr2 ){
    	DEBUGMSGTL(("mibII/hdsl2Shdsl","interval15Index: Error requesting\n"));
    	PDEBUG(0,"interval15Index: Error requesting");
		goto exit;
    }

    PDEBUG(0,"Request successfull, p =%p",p);
    p = (endp_int_payload*)comm_frame_payload(fr2);
    if( !p ){
		goto exit;
    }
    //	printf("wirePair: Get params info for %s: units(%d) loops(%d)\n",tbl[interface_ind].name,p->units,p->loops);
    perf_int = p->cntrs;
    name[vp->namelen+4] = p->int_num;
    ret_val = name[vp->namelen+4];
    *length = vp->namelen+5;
 exit:
    if( fr1 )
		comm_frame_free(fr1);
    if( fr2 )
		comm_frame_free(fr2);
    PDEBUG(0,"<----------------------------");
    return ret_val;
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
    endp_int_payload *p15,*p1d;
    char *return_ptr = NULL;
    int int_num;

    comm = init_comm();
    if(!comm){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
		return NULL;
    }

    if ( (int_num = header_intervalIndex(vp,name,length,exact,var_len,write_method,APP_ENDP_15MIN) )
		 == MATCH_FAILED )
        goto exit;
    
	//    printf("Result : if(%s) unit(%d) side(%d) pair(%d) int(%d)\n",tbl[interface_ind].name,unit_index,endp_index,wire_index,int_num);

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
    comm_free(comm);
    comm = NULL;
    PDEBUG(0,"<--------------------------");
    return return_ptr;
}


/*
 * Segment Endpoint 1-Day Interval Status/Performance Group
 */
u_char *
var_1DayIntervalEntry(struct variable * vp,
					  oid * name,
					  size_t * length,
					  int exact, size_t * var_len, WriteMethod ** write_method)
{
    endp_int_payload *p1d;
    char *return_ptr = NULL;
    int int_num;

	PDEBUG(0,"--------------------->");


    comm = init_comm();
    if(!comm){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
		return NULL;
    }

    if ( (int_num = header_intervalIndex(vp,name,length,exact,var_len,write_method,APP_ENDP_1DAY) )
		 == MATCH_FAILED )
        goto exit;
    
    PDEBUG(0,"(Result) : if(%s) unit(%d) side(%d) pair(%d) int(%d)",tbl[interface_ind].name,unit_index,endp_index,wire_index,int_num);

    //---- ack ----//
    switch (vp->magic) {
    case ENDP_1D_MONSECS:
		long_return = (u_long)perf_int.mon_sec;
		return_ptr = (u_char*)&long_return;
		break;
    case ENDP_1D_ES:
		long_return = (u_long)perf_int.es;
		return_ptr = (u_char*)&long_return;
		break;
    case ENDP_1D_SES:
		long_return = (u_long)perf_int.ses;
		return_ptr = (u_char*)&long_return;
		break;
    case ENDP_1D_CRC:
		long_return = (u_long)perf_int.crc;
		return_ptr = (u_char*)&long_return;
		break;
    case ENDP_1D_LOSWS:
		long_return = (u_long)perf_int.losws;
		return_ptr = (u_char*)&long_return;
		break;
    case ENDP_1D_UAS:
		long_return = (u_long)perf_int.uas;
		return_ptr = (u_char*)&long_return;
		break;
    default:
		break;
    }
 exit:
    comm_free(comm);
    comm = NULL;
    return return_ptr;
}

/*
 * header_wirePairIndex:
 * 	Defines propriate Wire pair index for incoming OID
 *	(by now only 1 pair supported)
 */

cprof_payload _cprof;

static int 
header_confProfIndex(struct variable *vp,
					 oid * name,
					 size_t * length,
					 int exact, size_t * var_len, WriteMethod ** write_method )
{
    int base_compare;
    int oid_min = (vp->namelen > *length) ? *length : vp->namelen;
    char profname[MAX_OID_LEN];
    *write_method = 0;
    *var_len = sizeof(long);    // default to 'long' results //
    struct app_frame *fr1 = NULL,*fr2 = NULL;
    char *b;
    cprof_payload *p;
    int i,l;


	PDEBUG(DERR,"start");
    if( (base_compare = snmp_oid_compare(name,oid_min,vp->name,oid_min)) > 0){
		// OID is grater than supported
		return MATCH_FAILED;
    }

	PDEBUG(DERR,"process...");
    
    if( exact ){
		int len;
		if( base_compare || (*length < vp->namelen) ){
			// Already incorrect
			return MATCH_FAILED;
		}
        len = *length - vp->namelen;
        len = (len>SNMP_ADMIN_LEN) ? SNMP_ADMIN_LEN : len;
        memcpy(profname,name+vp->namelen,len);
		profname[len+1] = 0;	
		p = (cprof_payload*)
			comm_alloc_request(APP_CPROF,APP_GET,"",&fr1);
    }else{
		if( (base_compare < 0) || (!base_compare && *length<=vp->namelen) ){
			memcpy((char*)name,vp->name,((int)vp->namelen)*sizeof(oid));
			*length = vp->namelen;
			profname[0] = 0;
			//printf("%s: nonexact, first request\n",__FUNCTION__);
		}else{
			int len = *length - vp->namelen;
			len = (len>SNMP_ADMIN_LEN) ? SNMP_ADMIN_LEN : len;
			for(i=0;i<len;i++){
				profname[i] = name[vp->namelen+i];
			}
			profname[len] = 0;
			*length = vp->namelen;	    
			//printf("%s: nonexact, profname=%s\n",__FUNCTION__,profname);
		}
		p = (cprof_payload*)
			comm_alloc_request(APP_CPROF,APP_GET_NEXT,"",&fr1);
    }	
    if( !p ){
		DEBUGMSGTL(("mibII/hdsl2Shdsl","Cannot allocate application frame"));
		if( fr1 )
			comm_frame_free(fr1);
		return MATCH_FAILED;
    }

    strncpy(p->pname,profname,SNMP_ADMIN_LEN+1);
	PDEBUG(DERR,"Request profile: %s",p->pname);
    fr2 = comm_request(comm,fr1);
    if( !fr2 ){
		PDEBUG(DERR,"Error requesting");
		if( fr1 )
			comm_frame_free(fr1);
		return MATCH_FAILED;
    }
	PDEBUG(DERR,"request successfull");
    
    p = (cprof_payload*)comm_frame_payload(fr2);
    _cprof = *p;
	PDEBUG(DERR,"get profile - %s,len=%d,vp->namelen=%d",p->pname,strnlen(p->pname,SNMP_ADMIN_LEN+1),vp->namelen);
    l = strnlen(p->pname,SNMP_ADMIN_LEN+1);
    for(i=0;i<l;i++){
		name[vp->namelen+i] = p->pname[i];
    }
    *length += strnlen(p->pname,SNMP_ADMIN_LEN+1);

printf("%s: name:\n",__FUNCTION__);
for(i=0;i<*length;i++){
	if( (name[i]>='a' && name[i]<='z') || (name[i]>='0' && name[i]<='9') || name[i]=='#' )
		printf("%c.",name[i]);
	else 
		printf("%d.",name[i]);
}
printf("\n");

    if( fr1 )
        comm_frame_free(fr1);
    if( fr2 )
        comm_frame_free(fr2);
    return !(MATCH_FAILED);
}

u_char *
var_SpanConfProfEntry(struct variable * vp,
					  oid * name,
					  size_t * length,
					  int exact, size_t * var_len, WriteMethod ** write_method)
{
    char *return_ptr = NULL;
    int i;

	PDEBUG(DERR,"start");
    comm = init_comm();
    if(!comm){
        DEBUGMSGTL(("mibII/hdsl2Shdsl","Error connecting to \"eocd\""));
        return NULL;
    }
	PDEBUG(DERR,"Connect=OK");    

printf("%s: name:\n",__FUNCTION__);
for(i=0;i<*length;i++){
	if( (name[i]>='a' && name[i]<='z') || (name[i]>='0' && name[i]<='9') || name[i]=='#' ){
		printf("%c.",name[i]);
	}else {
		printf("%d.",name[i]);
	}
}
printf("\n");

    if ( header_confProfIndex(vp,name,length,exact,var_len,write_method)
		 == MATCH_FAILED ){
		PDEBUG(DERR,"Header_search=FAIL");
        goto exit;
    }
	PDEBUG(DERR,"Header_search=OK");

    //---- ack ----//
    switch (vp->magic) {
    case CONF_WIRE_IFACE:
		long_return= _cprof.conf.wires;
		return_ptr = (u_char *)&long_return;
		PDEBUG(DINFO,"WIRE_IFACE: ret=%d", _cprof.conf.wires);
		break;
    case CONF_MIN_LRATE:
		long_return = _cprof.conf.rate;
		return_ptr = (u_char *)&long_return;
		PDEBUG(DINFO,"MIN_LRATE: ret=%d", _cprof.conf.wires);
		break;
    case CONF_MAX_LRATE:
		long_return = _cprof.conf.rate;
		return_ptr = (u_char *)&long_return;
		PDEBUG(DINFO,"MAX_LRATE: ret=%d", _cprof.conf.wires);
		break;
    case CONF_PSD:
		long_return= _cprof.conf.psd;
		return_ptr = (u_char *)&long_return;
		break;
		/*	
			case CONF_TRNSM_MODE:
			*var_len = sizeof(char);
			long_return = *((unsigned char*)&info->transm_mode);
			return_ptr = (u_char *)&long_return;
			break; */
    case CONF_REM_ENABLE:
		long_return = _cprof.conf.remote_cfg;
		return_ptr = (u_char *)&long_return;
		break;
    case CONF_PWR_FEED:
		long_return = _cprof.conf.power;
		return_ptr = (u_char *)&long_return;
		break;
    case CONF_CURR_DOWN:
		long_return = _cprof.conf.cur_marg_down;
		return_ptr = (u_char *)&long_return;
		break;
    case CONF_WORST_DOWN:
		long_return = _cprof.conf.worst_marg_down;
		return_ptr = (u_char *)&long_return;
		break;
    case CONF_CURR_UP:
		long_return = _cprof.conf.cur_marg_up;
		return_ptr = (u_char *)&long_return;
		break;
    case CONF_WORST_UP:
		long_return = _cprof.conf.worst_marg_up;
		return_ptr = (u_char *)&long_return;
		break;
		/*    case CONF_USED_MARG:
		 *var_len = sizeof(char);
		 long_return = *((unsigned char*)&info->used_margins);
		 return_ptr = (u_char *)&long_return; */
    case CONF_REF_CLK:
		long_return = _cprof.conf.clk;
		return_ptr = (u_char *)&long_return;
		break;
    case CONF_LPROBE:
		long_return = _cprof.conf.line_probe;
		return_ptr = (u_char *)&long_return;
		break;
		/*    case CONF_ROW_ST:
			  long_return = info->status;
			  return_ptr = (u_char *)&long_return;
		*/
    }
    
	PDEBUG(DERR,"exit:");
 exit:
    comm_free(comm);
    comm = NULL;    
	PDEBUG(DERR,"return");
    return return_ptr;
}


