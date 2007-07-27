#ifndef EOCD_APP_COMM_CLIH
#define EOCD_APP_COMM_CLI_H

#include <app_comm.h>

class app_comm_cli : public app_comm {
protected:
    int complete_wait();
public:
    app_comm_cli(char *sock_name);
    int send(int conn_num,char *buf,size_t size);
    ssize_t recv(int &conn_idx,char *&buf);
};

#endif


