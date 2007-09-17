/*
 * Note: this file originally auto-generated by mib2c using
 *        : mib2c.check_values.conf,v 1.5 2003/05/31 00:11:57 hardaker Exp $
 */

/********************************************************************
 *                       NOTE   NOTE   NOTE
 *   This file is auto-generated and SHOULD NOT BE EDITED by hand.
 *   Modify the netSnmpHostsTable_checkfns_local.[ch] files insead so that you
 *   can regenerate this one as mib2c improvements are made.
 ********************************************************************/

/*
 * standard headers 
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include "netSnmpHostsTable_checkfns.h"
#include "netSnmpHostsTable_checkfns_local.h"
#include "netSnmpHostsTable_enums.h"

/** Decides if an incoming value for the netSnmpHostAddressType mib node is legal.
 *  @param type    The incoming data type.
 *  @param val     The value to be checked.
 *  @param val_len The length of data stored in val (in bytes).
 *  @return 0 if the incoming value is legal, an SNMP error code otherwise.
 */
int
check_netSnmpHostAddressType(int type, long *val, size_t val_len,
                             long *old_val, size_t old_val_len)
{

    int             ret;

    /** Check to see that we were called legally */
    if (!val)
        return SNMP_ERR_GENERR;

    /** Check the incoming type for correctness */
    if (type != ASN_INTEGER)
        return SNMP_ERR_WRONGTYPE;

    /** Check the enums.  Legal values will continue, others return error. */
    switch (*val) {
    case NETSNMPHOSTADDRESSTYPE_UNKNOWN:
    case NETSNMPHOSTADDRESSTYPE_IPV4:
    case NETSNMPHOSTADDRESSTYPE_IPV6:
    case NETSNMPHOSTADDRESSTYPE_IPV4Z:
    case NETSNMPHOSTADDRESSTYPE_IPV6Z:
    case NETSNMPHOSTADDRESSTYPE_DNS:
        break;

    /** not a legal enum value.  return an error */
    default:
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    ret = SNMP_ERR_NOERROR;


    /** looks ok, call the local version of the same function. */
    return check_netSnmpHostAddressType_local(type, val, val_len, old_val,
                                              old_val_len);
}

/** Decides if an incoming value for the netSnmpHostAddress mib node is legal.
 *  @param type    The incoming data type.
 *  @param val     The value to be checked.
 *  @param val_len The length of data stored in val (in bytes).
 *  @return 0 if the incoming value is legal, an SNMP error code otherwise.
 */
int
check_netSnmpHostAddress(int type, char *val, size_t val_len,
                         char *old_val, size_t old_val_len)
{

    int             ret;

    /** Check to see that we were called legally */
    if (!val)
        return SNMP_ERR_GENERR;

    /** Check the incoming type for correctness */
    if (type != ASN_OCTET_STR)
        return SNMP_ERR_WRONGTYPE;

    /** Check the ranges of the passed value for legality */
    if (!(val_len >= 0 && val_len <= 255)
        ) {
        return SNMP_ERR_WRONGVALUE;
    }


    /** looks ok, call the local version of the same function. */
    return check_netSnmpHostAddress_local(type, val, val_len, old_val,
                                          old_val_len);
}

/** Decides if an incoming value for the netSnmpHostStorage mib node is legal.
 *  @param type    The incoming data type.
 *  @param val     The value to be checked.
 *  @param val_len The length of data stored in val (in bytes).
 *  @return 0 if the incoming value is legal, an SNMP error code otherwise.
 */
int
check_netSnmpHostStorage(int type, long *val, size_t val_len,
                         long *old_val, size_t old_val_len)
{

    int             ret;

    /** Check to see that we were called legally */
    if (!val)
        return SNMP_ERR_GENERR;

    /** Check the incoming type for correctness */
    if (type != ASN_INTEGER)
        return SNMP_ERR_WRONGTYPE;

    /** Check the enums.  Legal values will continue, others return error. */
    switch (*val) {
    case NETSNMPHOSTSTORAGE_OTHER:
    case NETSNMPHOSTSTORAGE_VOLATILE:
    case NETSNMPHOSTSTORAGE_NONVOLATILE:
    case NETSNMPHOSTSTORAGE_PERMANENT:
    case NETSNMPHOSTSTORAGE_READONLY:
        break;

    /** not a legal enum value.  return an error */
    default:
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    ret = SNMP_ERR_NOERROR;

    if (ret =
        check_storage_transition((old_val) ? *old_val : SNMP_STORAGE_NONE,
                                 *val))
        return ret;

    /** looks ok, call the local version of the same function. */
    return check_netSnmpHostStorage_local(type, val, val_len, old_val,
                                          old_val_len);
}

/** Decides if an incoming value for the netSnmpHostRowStatus mib node is legal.
 *  @param type    The incoming data type.
 *  @param val     The value to be checked.
 *  @param val_len The length of data stored in val (in bytes).
 *  @return 0 if the incoming value is legal, an SNMP error code otherwise.
 */
int
check_netSnmpHostRowStatus(int type, long *val, size_t val_len,
                           long *old_val, size_t old_val_len)
{

    int             ret;

    /** Check to see that we were called legally */
    if (!val)
        return SNMP_ERR_GENERR;

    /** Check the incoming type for correctness */
    if (type != ASN_INTEGER)
        return SNMP_ERR_WRONGTYPE;

    /** Check the enums.  Legal values will continue, others return error. */
    switch (*val) {
    case NETSNMPHOSTROWSTATUS_ACTIVE:
    case NETSNMPHOSTROWSTATUS_NOTINSERVICE:
    case NETSNMPHOSTROWSTATUS_NOTREADY:
    case NETSNMPHOSTROWSTATUS_CREATEANDGO:
    case NETSNMPHOSTROWSTATUS_CREATEANDWAIT:
    case NETSNMPHOSTROWSTATUS_DESTROY:
        break;

    /** not a legal enum value.  return an error */
    default:
        return SNMP_ERR_INCONSISTENTVALUE;
    }
    ret = SNMP_ERR_NOERROR;

    if (ret =
        check_rowstatus_transition((old_val) ? *old_val : RS_NONEXISTENT,
                                   *val))
        return ret;

    /** looks ok, call the local version of the same function. */
    return check_netSnmpHostRowStatus_local(type, val, val_len, old_val,
                                            old_val_len);
}
