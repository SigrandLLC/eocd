#ifndef EOCD_APP_COMM_CLI_H
#define EOCD_APP_COMM_CLI_H

#include <app-if/app_comm.h>

class app_comm_cli : public app_comm {
protected:
    int complete_wait();
public:
    app_comm_cli(char *sock_name,int quiet = 0);
    ~app_comm_cli();
    int send(char *buf,size_t size);
    ssize_t recv(char *&buf);
};

#endif


