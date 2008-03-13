#ifndef EOC_MAIN_H
#define EOC_MAIN_H

#include <conf_profile.h>
#include <channel.h>
#include <utils/hash_table.h>
#include <snmp/snmp-generic.h>
#include <app-if/app_frame.h>
#include <app-if/app_comm_srv.h>


// TODO: consult for OS about this values
#define MAX_IF_NAME_LEN 256
#define MAX_FNAME 256

#define TICK_PER_MINUTE_MAX 60
#define TICK_PER_MINUTE_DEF 6

#define OS_IF_PATH "/sys/class/net"



class EOC_main{
 protected:
 	class if_correction{
	public:
		char oif[IF_NAME_LEN],nif[IF_NAME_LEN];
	};
    char config_file[MAX_FNAME];
    char correct_file[MAX_FNAME];	
    hash_table conf_profs;
    hash_table alarm_profs;
    hash_table channels;
    app_comm_srv app_srv;
    int valid;
	int tick_per_min;
	channel_elem *err_cfgchans[SPAN_NAMES_NUM];
	int err_cfgchans_cnt;
 public:
 EOC_main(char *cfg,char *sockpath,char *correct) : conf_profs(SNMP_ADMIN_LEN), alarm_profs(SNMP_ADMIN_LEN),
		channels(MAX_IF_NAME_LEN), app_srv(sockpath,"eocd-socket")
		{
			valid = 0;
			strncpy(config_file,cfg,MAX_FNAME);
			strncpy(correct_file,cfg,MAX_FNAME);
			config_file[MAX_FNAME-1] = '\0';
			if( read_config() )
				return;
			if( correct_config() ){
				write_config();
			}
			configure_channels();
			valid = 1;
		}
    ~EOC_main(){
    }
    int get_valid(){ return valid; }
    // Serving configuration file
	int count_lines(char *fname);
	int copy_word(char *src,int ssize,char *dst,int dsize);
    int read_config();
	int write_config();
	int correct_config();
	
    // Add (initialise) or change slave channel with name "ch_name"
    int add_slave(char *ch_name,char *conf,int app_cfg=1);
    // Add (initialise) or change master channel with name "ch_name"
    int add_master(char *ch_name, char *conf,char *alarm,int reps,int tick,int app_cfg=1);
    //
    int configure_channels();    
    // process channels
    int poll_channels();

    void app_listen();
    int app_request(app_frame *fr);
    int app_spanname(app_frame *fr);
    int app_spanconf(app_frame *fr);
    int app_chann_request(app_frame *fr);
    int app_cprof(app_frame *fr);
    int app_list_cprof(app_frame *fr);
	int app_add_cprof(app_frame *fr);
	int app_del_cprof(app_frame *fr);
	int app_add_chan(app_frame *fr);
	int app_del_chan(app_frame *fr);
	int app_chng_chan(app_frame *fr);
    int app_endpalarm_prof(app_frame *fr);

};


#endif
