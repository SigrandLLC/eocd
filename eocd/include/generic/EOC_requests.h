#ifndef EOC_REQUESTS_H
#define EOC_REQUESTS_H

#difine REQ_DISCOVERY 1
typedef struct{
    unsigned char hop;
} req_discovery; 
#difine REQ_DISCOVERY_SZ sizeof(req_discovery);

#endif