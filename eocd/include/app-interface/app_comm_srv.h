#ifndef EOC_APP_COMM_SRV_H
#define EOC_APP_COMM_SRV_H

#include <app-interface/app_comm.h>

class app_comm_srv : public app_comm {
protected:
    #define MAX_CONNECTIONS 5
    int conn_fd[MAX_CONNECTIONS];
    unsigned char conn_act[MAX_CONNECTIONS];
    int conn_num;

    // functions
    int complete_wait();
    int new_connection();
    void build_select_list();
    int next_fd();
public:
    app_comm_srv(char *sock_path,char *sock_name);
    int send(int conn_num,char *buf,size_t size);
    ssize_t recv(int &conn_idx,char *&buf);
};

#endif
