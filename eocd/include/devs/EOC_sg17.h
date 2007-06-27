#ifndef SIGRAND_EOC_SG17_H
#define SIGRAND_EOC_SG17_H

#include <devs/EOC_dev.h>
#include <generic/EOC_msg.h>

#define HDLC_BUFF_SZ 112

class EOC_sg17 : public EOC_dev{
protected:
    char *fname;
    int valid;    
public:
    EOC_sg17(char *fname);
    ~EOC_sg17();    
    int send(EOC_msg *m);
    EOC_msg *recv();
};

#endif
