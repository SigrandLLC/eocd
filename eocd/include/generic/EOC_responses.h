#ifndef EOC_RESPONSES_H
#define EOC_RESPONSES_H

#include <generic/EOC_requests.h>
#define RESP_OFFSET 128

typedef struct{
    char hop;
    char res1;
    char vendor_id[8];
    char eoc_softw_ver;
    char shdsl_ver;
    char fwd_loss:1;
    char :7;
} resp_discovery;
#define RESP_DISCOVERY (REQ_DISCOVERY+RESP_OFFSET)
#define RESP_DISCOVERY_SZ sizeof(resp_discovery);

#endif