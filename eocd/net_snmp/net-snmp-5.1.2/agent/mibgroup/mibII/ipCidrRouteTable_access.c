
/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.access_functions.conf$
 */

#include <net-snmp/net-snmp-config.h>
#include "route_headers.h"
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "ipCidrRouteTable_access.h"
#include "ipCidrRouteTable_enums.h"

/*
 * NOTE:
 * - these GET routines MUST return freshly malloced data and must not
 * return a pointer which is still in use somewhere else.
 * - these SET routines must copy the incoming data and can not take
 * ownership of the memory passed in by the val pointer.
 */

/** a global static we'll make use of a lot to map to the right
   datatype to return (which for SNMP integer's is always a long). */
static u_long long_ret;


/*
 * User-defined data access functions for data in table ipCidrRouteTable
 */

u_long         *
get_ipCidrRouteDest(void *data_context, size_t * ret_len)
{
    RTENTRY *ourroute = (RTENTRY *) data_context;
    long_ret = ((struct sockaddr_in *) (&ourroute->rt_dst))->sin_addr.s_addr;
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

u_long         *
get_ipCidrRouteMask(void *data_context, size_t * ret_len)
{
    RTENTRY *ourroute = (RTENTRY *) data_context;
    long_ret = ((struct sockaddr_in *) (&ourroute->rt_genmask))->sin_addr.s_addr;
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

long           *
get_ipCidrRouteTos(void *data_context, size_t * ret_len)
{
    RTENTRY *ourroute = (RTENTRY *) data_context;
    long_ret = ourroute->rt_tos;
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

u_long         *
get_ipCidrRouteNextHop(void *data_context, size_t * ret_len)
{
    RTENTRY *ourroute = (RTENTRY *) data_context;
    long_ret = ((struct sockaddr_in *) (&ourroute->rt_gateway))->sin_addr.s_addr;
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

long           *
get_ipCidrRouteIfIndex(void *data_context, size_t * ret_len)
{
    RTENTRY *ourroute = (RTENTRY *) data_context;
    long_ret = ourroute->rt_unit;
    *ret_len = sizeof(long_ret);;
    return &long_ret;
}

int
set_ipCidrRouteIfIndex(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

long           *
get_ipCidrRouteType(void *data_context, size_t * ret_len)
{
    RTENTRY *ourroute = (RTENTRY *) data_context;

    if (ourroute->rt_flags & RTF_UP) {
        if (ourroute->rt_flags & RTF_GATEWAY) {
            long_ret = IPCIDRROUTETYPE_REMOTE;
        } else {
            long_ret = IPCIDRROUTETYPE_LOCAL;
        }
    } else {
        long_ret = IPCIDRROUTETYPE_REJECT;
    }
    *ret_len = sizeof(long_ret);
    return &long_ret;

}

int
set_ipCidrRouteType(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

long           *
get_ipCidrRouteProto(void *data_context, size_t * ret_len)
{
    RTENTRY *ourroute = (RTENTRY *) data_context;
    /* XXX: this is wacked */
    long_ret = (ourroute->rt_flags & RTF_DYNAMIC)
        ? IPCIDRROUTEPROTO_ICMP : IPCIDRROUTEPROTO_LOCAL;
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

long           *
get_ipCidrRouteAge(void *data_context, size_t * ret_len)
{
    long_ret = 0; /* we don't know, and defval = 0 */
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

oid            *
get_ipCidrRouteInfo(void *data_context, size_t * ret_len)
{
    static oid zerodotzero[2] = { 0, 0 };
    *ret_len = sizeof(zerodotzero);
    return zerodotzero;
}

int
set_ipCidrRouteInfo(void *data_context, oid * val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

/** XXX: return a data pointer to the data for the ipCidrRouteNextHopAS column and set
         ret_len to its proper size in bytes. */
long           *
get_ipCidrRouteNextHopAS(void *data_context, size_t * ret_len)
{
    long_ret = 0; /* we don't know, and defval = 0 */
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

int
set_ipCidrRouteNextHopAS(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

long           *
get_ipCidrRouteMetric1(void *data_context, size_t * ret_len)
{
    RTENTRY *ourroute = (RTENTRY *) data_context;
    long_ret = ourroute->rt_metric;
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

int
set_ipCidrRouteMetric1(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

long           *
get_ipCidrRouteMetric2(void *data_context, size_t * ret_len)
{
    long_ret = -1; /* unused */
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

 /** XXX: Set the value of the ipCidrRouteMetric2 column and return
          SNMPERR_SUCCESS on SNMPERR_GENERR on failure. */
int
set_ipCidrRouteMetric2(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

/** XXX: return a data pointer to the data for the ipCidrRouteMetric3 column and set
         ret_len to its proper size in bytes. */
long           *
get_ipCidrRouteMetric3(void *data_context, size_t * ret_len)
{
    long_ret = -1; /* unused */
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

 /** XXX: Set the value of the ipCidrRouteMetric3 column and return
          SNMPERR_SUCCESS on SNMPERR_GENERR on failure. */
int
set_ipCidrRouteMetric3(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

/** XXX: return a data pointer to the data for the ipCidrRouteMetric4 column and set
         ret_len to its proper size in bytes. */
long           *
get_ipCidrRouteMetric4(void *data_context, size_t * ret_len)
{
    long_ret = -1; /* unused */
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

 /** XXX: Set the value of the ipCidrRouteMetric4 column and return
          SNMPERR_SUCCESS on SNMPERR_GENERR on failure. */
int
set_ipCidrRouteMetric4(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

/** XXX: return a data pointer to the data for the ipCidrRouteMetric5 column and set
         ret_len to its proper size in bytes. */
long           *
get_ipCidrRouteMetric5(void *data_context, size_t * ret_len)
{
    long_ret = -1; /* unused */
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

 /** XXX: Set the value of the ipCidrRouteMetric5 column and return
          SNMPERR_SUCCESS on SNMPERR_GENERR on failure. */
int
set_ipCidrRouteMetric5(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}

/** XXX: return a data pointer to the data for the ipCidrRouteStatus column and set
         ret_len to its proper size in bytes. */
long           *
get_ipCidrRouteStatus(void *data_context, size_t * ret_len)
{
    /* the only value supported for real routes */
    long_ret = IPCIDRROUTESTATUS_ACTIVE;
    *ret_len = sizeof(long_ret);
    return &long_ret;
}

 /** XXX: Set the value of the ipCidrRouteStatus column and return
          SNMPERR_SUCCESS on SNMPERR_GENERR on failure. */
int
set_ipCidrRouteStatus(void *data_context, long *val, size_t val_len)
{
    return SNMP_ERR_NOERROR;      /** XXX: change if an error occurs */
}
