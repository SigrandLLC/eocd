/*
 * This file was generated by mib2c and is intended for use as a mib module
 * for the ucd-snmp snmpd agent.
 */


#ifndef _MIBGROUP_SNMPNOTIFYFILTERPROFILETABLE_H
#define _MIBGROUP_SNMPNOTIFYFILTERPROFILETABLE_H


/*
 * we may use header_complex from the header_complex module
 */


config_require(header_complex)


    /*
     * our storage structure(s)
     */
     struct snmpNotifyFilterProfileTable_data {

         char           *snmpTargetParamsName;
         size_t          snmpTargetParamsNameLen;
         char           *snmpNotifyFilterProfileName;
         size_t          snmpNotifyFilterProfileNameLen;
         long            snmpNotifyFilterProfileStorType;
         long            snmpNotifyFilterProfileRowStatus;

     };




/*
 * enum definitions from the covered mib sections
 */








/*
 * function prototypes
 */


     void            init_snmpNotifyFilterProfileTable(void);
     FindVarMethod   var_snmpNotifyFilterProfileTable;
     void            parse_snmpNotifyFilterProfileTable(const char *,
                                                        char *);
     SNMPCallback    store_snmpNotifyFilterProfileTable;

     WriteMethod     write_snmpNotifyFilterProfileName;
     WriteMethod     write_snmpNotifyFilterProfileStorType;
     WriteMethod     write_snmpNotifyFilterProfileRowStatus;



#endif                          /* _MIBGROUP_SNMPNOTIFYFILTERPROFILETABLE_H */
