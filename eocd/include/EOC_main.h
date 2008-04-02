#ifndef EOC_MAIN_H
#define EOC_MAIN_H

#include <conf_profile.h>
#include <channel.h>
#include <utils/hash_table.h>
#include <snmp/snmp-generic.h>
#include <app-if/app_frame.h>
#include <app-if/app_comm_srv.h>
#include <list>
using namespace std;

#define TICK_PER_MINUTE_MAX 60
#define TICK_PER_MINUTE_DEF 12


class EOC_main{
 public:
 	typedef struct{
		int pcislot,pcidev;
		char *cprof,*aprof;
		int reps,can_apply,master;
		int pbomode;
		char pboval[PBO_SETTING_LEN];
	} dev_settings_t;
 
 protected:
    char config_file[FNAME_SIZE];
    hash_table conf_profs;
    hash_table alarm_profs;
    hash_table channels;
	// backup settings for not existed devices for future use
	list<dev_settings_t> nexist_devs;
	
    app_comm_srv app_srv;
    int valid;
	int tick_per_min;
	channel_elem *err_cfgchans[SPAN_NAMES_NUM];
	int err_cfgchans_cnt;
 public:
 EOC_main(char *cfg,char *sockpath) : conf_profs(SNMP_ADMIN_LEN), alarm_profs(SNMP_ADMIN_LEN),
		channels(IF_NAME_LEN), app_srv(sockpath,"eocd-socket")
		{
			valid = 0;
			strncpy(config_file,cfg,FNAME_SIZE);
			config_file[FNAME_SIZE-1] = '\0';
			if( read_config() )
				return;
			configure_channels();
			valid = 1;
		}
    ~EOC_main(){
    }
    int get_valid(){ return valid; }
    // Read configuration file and initialise or change channels
    int read_config();
	int write_config();
    // Write configuration to config file (when it changes from network)
	//    int write_config();
    // Add (initialise) or change slave channel with name "ch_name"
    int add_slave(char *ch_name,char *conf,int app_cfg=1);
    // Add (initialise) or change master channel with name "ch_name"
    int add_master(char *ch_name, char *conf,char *alarm,int reps,int tick,int pbomode,char *pboval,int app_cfg=1);
	// Add device to unexistence group
    int add_nexist_slave(int pcislot,int pcidev,char *conf,int app_cfg=1);
    int add_nexist_master(int pcislot,int pcidev,char *conf,char *alarm,int reps,int pbomode,char *pboval,int app_cfg=1);
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
	int app_chan_pbo(app_frame *fr);
    int app_endpalarm_prof(app_frame *fr);

};


#endif
