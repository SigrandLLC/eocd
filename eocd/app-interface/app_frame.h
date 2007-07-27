#ifndef EOCD_FRAME_H
#define EOCD_FRAME_H

#include <

class app_frame{
private:
typedef struct{
    unsigned char id;
    time_t tstamp;
    unsigned int len;
}app_frame_header;

    char *buf;
    int size;
    


}

#endif