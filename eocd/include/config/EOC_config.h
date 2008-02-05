#ifndef EOC_CONFIG_H
#define EOC_CONFIG_H

#include <generic/EOC_types.h>
#include <utils/hash_table.h>

#include <conf_profile.h>

class EOC_config{
    hash_table *conf_prof;
    hash_table *alarm_prof;
    char *cprof_name,*cprof_name_old;
    char *aprof_name,*aprof_name_old;
    u16 rep_num;
    int app_cfg;
 public:
    EOC_config(hash_table *c,hash_table *a,char *cn,char *an,u16 rep,int _app_cfg){
		conf_prof = c;
		alarm_prof = a;
		cprof_name = cn;
		cprof_name_old = NULL;
		aprof_name = an;
		aprof_name_old = NULL;
		rep_num = rep;
		app_cfg = _app_cfg;
	}
    EOC_config(hash_table *c,char *cn,int _app_cfg){
		conf_prof = c;
		alarm_prof = NULL;
		cprof_name = cn;
		cprof_name_old = NULL;
		aprof_name = NULL;
		aprof_name_old = NULL;
		app_cfg = _app_cfg;
	}

    ~EOC_config(){
		if( cprof_name )
			free(cprof_name);
		if( cprof_name_old )
			free(cprof_name_old);
		if( aprof_name )
			free(aprof_name);
		if( aprof_name_old )
			free(aprof_name_old);
	}

    const char *cprof(){return cprof_name;}
    const char *aprof(){return aprof_name;}
    int cprof(char *p){
		if( cprof_name_old)
			free(cprof_name_old);
		cprof_name_old = cprof_name;
		cprof_name = p;
	}
	int cprof_revert(){
		if( cprof_name_old ){
			free(cprof_name);
			cprof_name = cprof_name_old;
			cprof_name_old = NULL;
			return 0;
		}
		return -1;
	}
    int aprof(char *p){
		if( aprof_name_old )
			free(aprof_name_old);
		aprof_name_old = aprof_name;
		aprof_name = p;
	}
	int aprof_revert(){
		if( aprof_name_old ){
			free(aprof_name);
			aprof_name = aprof_name_old;
			aprof_name_old = NULL;
			return 0;
		}
		return -1;
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
		if( !conf_prof )
			return NULL;
		return conf_prof->find(cprof_name,strlen(cprof_name));
	}
    hash_elem *alarm(){
		if( !alarm_prof )
			return NULL;
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
