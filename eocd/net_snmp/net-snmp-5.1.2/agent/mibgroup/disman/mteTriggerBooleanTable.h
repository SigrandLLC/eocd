/*
 * This file was generated by mib2c and is intended for use as a mib module
 * for the ucd-snmp snmpd agent.
 */


#ifndef _MIBGROUP_MTETRIGGERBOOLEANTABLE_H
#define _MIBGROUP_MTETRIGGERBOOLEANTABLE_H


/*
 * we may use header_complex from the header_complex module
 */


config_require(header_complex)
    /*
     * enum definitions from the covered mib sections
     */
#define MTETRIGGERBOOLEANCOMPARISON_UNEQUAL      1
#define MTETRIGGERBOOLEANCOMPARISON_EQUAL        2
#define MTETRIGGERBOOLEANCOMPARISON_LESS         3
#define MTETRIGGERBOOLEANCOMPARISON_LESSOREQUAL  4
#define MTETRIGGERBOOLEANCOMPARISON_GREATER      5
#define MTETRIGGERBOOLEANCOMPARISON_GREATEROREQUAL 6
#define MTETRIGGERBOOLEANSTARTUP_TRUE            1
#define MTETRIGGERBOOLEANSTARTUP_FALSE           2
    /*
     * function prototypes
     */
     void            init_mteTriggerBooleanTable(void);
     FindVarMethod   var_mteTriggerBooleanTable;

     WriteMethod     write_mteTriggerBooleanComparison;
     WriteMethod     write_mteTriggerBooleanValue;
     WriteMethod     write_mteTriggerBooleanStartup;
     WriteMethod     write_mteTriggerBooleanObjectsOwner;
     WriteMethod     write_mteTriggerBooleanObjects;
     WriteMethod     write_mteTriggerBooleanEventOwner;
     WriteMethod     write_mteTriggerBooleanEvent;




#endif                          /* _MIBGROUP_MTETRIGGERBOOLEANTABLE_H */
