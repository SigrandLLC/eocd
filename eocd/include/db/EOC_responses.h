#ifndef EOC_RESPONSES_H
#define EOC_RESPONSES_H

#define DISCOVERY_RESPONSE_SZ 13
typedef struct{
    char hop;
    char res1;
    char vendor_id[8];
    char eoc_softw_ver;
    char shdsl_ver;
    char fwd_loss:1;
    char :7;
} resp_discovery;

#endif