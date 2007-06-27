#ifndef SIGRAND_EOC_SRU_H
#define SIGRAND_EOC_SRU_H

#include "EOC_dev.h"
#include "EOC_msg.h"
extern "C" {
#include "../include/sdfe4_lib.h"
}

#define HDLC_MSG_SIZE 112

class EOC_sru : public EOC_dev{
protected:
    struct sdfe4 *hwdev;
    int ch;
public:
    EOC_sru(struct sdfe4 *h,int c);
    int send(EOC_msg *m);
    int recv(EOC_msg *m);
    int init(char *ptr,int size);
};

#endif
