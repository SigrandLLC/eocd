#ifndef SIGRAND_EOC_DEV_MASTER_H
#define SIGRAND_EOC_DEV_MASTER_H

#include <span_profile.h>
#include <devs/EOC_dev.h>


class EOC_dev_terminal: public EOC_dev{
protected:
    char *ifname;
    char *conf_path;
    char *chan_path;
    int check_ctrl_files(char *d,char **opts,int opts_num);
public:
    virtual  span_conf_profile_t *cur_config() = 0;                                                                                     
    virtual int configure(span_conf_profile_t &cfg) = 0; 
    virtual int configure() = 0; 
};
 

#endif
