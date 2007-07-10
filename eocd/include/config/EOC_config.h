#ifndef EOC_CONFIG_H
#define EOC_CONFIG_H

#include <generic/EOC_generic.h>

class EOC_config{
protected:
    int a;
    char cfg_file[256];
public:
    EOC_config(char *file){ strncpy(cfg_file,file,256); }
    char snr_marg(unit u){ return 10; }
    char loop_attn(unit u) { return 10; }
};

#endif
