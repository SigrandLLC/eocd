#ifndef EOC_CONFIG_H
#define EOC_CONFIG_H

#include <generic/EOC_types.h>
#include <utils/hash_table.h>

#include <conf_profile.h>

class EOC_config{
    hash_table *conf_prof;
    hash_table *alarm_prof;
    char *cprof_name;
    char *aprof_name;
    u16 rep_num;
    int app_cfg;
 public:
    EOC_config(hash_table *c,hash_table *a,char *cn,char *an,u16 rep,int _app_cfg){
		conf_prof = c;
		alarm_prof = a;
		cprof_name = cn;
		aprof_name = an;
		rep_num = rep;
		app_cfg = _app_cfg;
	}
    const char *cprof(){return cprof_name;}
    const char *aprof(){return aprof_name;}
    int cprof(char *p){
		free(cprof_name);
		cprof_name = p;
	}
    int aprof(char *p){
		free(aprof_name);
		aprof_name = p;
	}

    u16 repeaters(){ return rep_num; }
    int repeaters(u16 rnum){ 
		if( rnum <= MAX_REPEATERS ){
			rep_num = rnum;
			return 0;
		}
		return -1;
	}
    hash_elem *conf(){
		return conf_prof->find(cprof_name,strlen(cprof_name));
	}
    hash_elem *alarm(){
		return alarm_prof->find(aprof_name,strlen(aprof_name));
	}
    s8 snr_tresh(){ return 0;}
    s8 loop_tresh(){ return 0; }
    int can_apply(){ return app_cfg; }
    int can_apply(u8 _app){ 
		app_cfg = _app;
	}

};

#endif
