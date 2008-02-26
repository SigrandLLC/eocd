#ifndef SIGRAND_EOC_DEV_MASTER_H
#define SIGRAND_EOC_DEV_MASTER_H

#include <generic/span_conf_type.h>
#include <devs/EOC_dev.h>


class EOC_dev_terminal: public EOC_dev{
protected:
    char *ifname;
    char *conf_path;
    char *chan_path;
    int check_ctrl_files(char *d,char **opts,int opts_num);
public:
    virtual int cur_config(span_conf_profile_t &cfg,int &mode) = 0;
    virtual int configure(span_conf_profile_t &cfg,int t) = 0; 
};
 

#endif
