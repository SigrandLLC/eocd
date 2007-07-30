#ifndef EOCD_APP_COMM_CLIH
#define EOCD_APP_COMM_CLI_H

#include <app-interface/app_comm.h>

class app_comm_cli : public app_comm {
protected:
    int complete_wait();
public:
    app_comm_cli(char *sock_name);
    int send(char *buf,size_t size);
    ssize_t recv(char *&buf);
};

#endif


