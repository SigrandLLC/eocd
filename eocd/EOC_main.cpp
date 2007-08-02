// system includes
#include <sys/types.h>
#include <dirent.h>

// lib includes
#include <iostream>
#include <libconfig.h++>

// local includes
#include <generic/EOC_generic.h>
#include <snmp/snmp-generic.h>
#include <utils/hash_table.h>
#include <config/EOC_config.h>

#include <devs/EOC_dev.h>
#include <devs/EOC_dev_terminal.h>
#include <devs/EOC_mr17h.h>
#include <devs/EOC_mr16h.h>

#include <EOC_main.h>
#include <span_profile.h>
#include <channel.h>

using namespace libconfig;
using namespace std;

EOC_dev_terminal *
init_dev(char *name);


EOC_dev_terminal *
init_dev(char *name)
{
    char *path=OS_IF_PATH;
    DIR *dir;
    struct dirent *ent;
    EOC_dev_terminal *dev = NULL;
    
    if( !(dir = opendir(path) ) ){
	printf("Cannot open %s\n",path);
	return NULL;
    }

    while( (ent = readdir(dir)) ){
	if( !strcmp(ent->d_name,name) ){
	    printf("IF %s is phisicaly present\n",name);
	    char cfg_dir[PATH_SIZE];
	    DIR *dir1;
	    mr17h_conf_dir(ent->d_name,cfg_dir,PATH_SIZE);
	    if( dir1 = opendir(cfg_dir) ){
		printf("Seems it mr17h\n");
		dev = (EOC_dev_terminal*)new EOC_mr17h(name);
		closedir(dir1);
		if( !dev->init_ok() ){
		    printf("Error initialising %s\n",name);		    
		    delete dev;
		    return NULL;
		}
		printf("Dev %s successfully initialised\n",name);		    
		return dev; 
	    }
/*	    else { 
	        snprintf(cfg_dir,256,"/sys/bus/pci/drivers/sg16lan/%s",ent->d_name);
		if( dir1 = opendir(cfg_dir) ){
		    dev = new EOC_mr16h(name);
		    closedir(dir1);
		    if( !dev.init_ok() ){
			delete dev;
			return NULL;
		    }
		    return dev; 
		}
	    }
*/
	}
    }
    closedir(dir);
    return NULL;
}


int EOC_main::
read_config()
{
    Config cfg;
    char *name;
    int i;
    int tick_per_min = TICK_PER_MINUTE_DEF;

    //----------- Open config file ------------------//
    try
    {
	cfg.readFile((const char *)config_file);
    }catch(ParseException& ex){
	// eoc_debug("Error in %s (%d): %s",file,ex.getLine(),ex.getError());
	cout << "failed in line " << ex.getLine()<< " error: " << ex.getError() << endl;
	return -1;
    }catch(FileIOException &fex){
	// eoc_debug("Cannot open configuration file: %s",file);
	printf("Cannot open configuration file: %s\n",config_file);
	return -1;
    }catch(...){
	return -1;
    }

    //----------- read tick per minute value ------------------//
    // 1. Check that conf_profile group exist
    try{
	tick_per_min = cfg.lookup("act_per_minute");
	if( tick_per_min < 0 ){
	    //eoc_log("(%s): value of \"act_per_minute\" must be greater than zero",config_file);
	    printf("(%s): value of \"act_per_minute\" must be greater than zero\n",config_file);
	    return -1;
	}else if( tick_per_min > TICK_PER_MINUTE_MAX ){
	    //eoc_log("(%s): exceed maximum value of \"act_per_minute\": %d, must be %d",config_file,tick_per_min,TICK_PER_MINUTE_MAX);
	    printf("(%s): exceed maximum value of \"act_per_minute\": %d, must be %d\n",config_file,tick_per_min,TICK_PER_MINUTE_MAX);
	    return -1;
	}	
    }catch(SettingNotFoundException &snfex){
	// nothing to do - use default value
    }catch(SettingTypeException &tex){
	// wrong type - configuration parsing error
	//eoc_log("(%s): wrong data type of \"act_per_minute\" setting",config_file);
	printf("(%s): wrong data type of \"act_per_minute\" setting\n",config_file);
	return -1;
    }catch(...){
	//eoc_log("Unexpected error while parsing \"act_per_minute\" setting in %s",config_file);
	printf("Unexpected error while parsing  \"act_per_minute\" setting in %s\n",config_file);
	return -1;
    }
    
    cout << "tick_p_min = " << tick_per_min << endl;

    //----------- read span configuration profiles ------------------//
    // 1. Check that conf_profile group exist
    try{
	Setting &s = cfg.lookup("span_profiles");
    }catch(...){
        // eoc_log("(%s): cannot found \"conf_profiles\" section",config_file);
        printf("(%s): cannot found \"span_profiles\" section\n",config_file);
        return -1;
    }

    // 2. read all elements of conf_profiles
    for(i=0;1;i++){
	name = NULL;
	try{
    	    Setting &s = cfg.lookup("span_profiles");
	    const char *str = s[i]["name"];
	    if( !(name = strndup(str,SNMP_ADMIN_LEN)) ){
		//eoc_log("Not enougth memory");
		printf("Not enougth memory\n");
		return -1;
	    }
	    
	    int wires = s[i]["wires"];
	    int min_rate = s[i]["minRate"];
	    int max_rate = s[i]["maxRate"];
	    int annex = s[i]["annex"];
	    int power = s[i]["powerSource"];
	    int ref_clock = s[i]["refClock"];
	    int line_probe = s[i]["lineProbe"];
	    int cur_marg_down = s[i]["currCondMargDown"];
	    int worst_marg_down = s[i]["worstCaseMargDown"];
	    int cur_marg_up = s[i]["currCondMargUp"];
	    int worst_marg_up = s[i]["worstCaseMargUp"];

	    int use_cur_down;
	    int use_worst_down;
	    int use_cur_up;
	    int use_worst_up;

	    try{ use_cur_down = s[i]["useCurrCondMargDown"];
	    }catch(...){ use_cur_down = 0; }

	    try{ use_worst_down = s[i]["useWorseCaseMargDown"];
	    }catch(...){ use_worst_down = 0; }

	    try{ use_cur_up = s[i]["useCurrCondMargUp"];
	    }catch(...){ use_cur_up = 0; }

	    try{ use_worst_up = s[i]["useWorseCaseMargUp"];
	    }catch(...){ use_worst_up = 0; }

	    
	    // Check input values
	    if( wires > 4 || wires <= 0 ){
		//eoc_log("(%s): wrong \"wires\" value in %s profile: %d , may be 1-4",config_file,name,wires);
		printf("(%s): wrong \"wires\" value in %s profile: %d , may be 1-4\n",config_file,name,wires);
		return -1;
	    }
	    if( annex > 2 || annex <= 0){
		//eoc_log("(%s): wrong \"annex\" value in %s profile: %d , may be 0-3",config_file,name,annex);
		printf("(%s): wrong \"wires\" value in %s profile: %d , may be 1-4\n",config_file,name,annex);
		return -1;
	    }
	    	
	    if( power > 3 && power <= 0){
		//eoc_log("(%s): wrong \"power\" value in %s profile: %d , may be 0,1",config_file,name,power);
		printf("(%s) wrong \"power\" value in %s profile: %d , may be 0,1\n",config_file,name,power);
		return -1;
	    }

	    if( ref_clock > 4 || ref_clock <= 0){
		//eoc_log("(%s): wrong \"ref_clock\" value in %s profile: %d , may be 0-3",config_file,name,ref_clock);
		printf("(%s): wrong \"ref_clock\" value in %s profile: %d , may be 1-4\n",config_file,name,ref_clock);
		return -1;
	    }
	    	
	    if( line_probe != 1 && line_probe != 2){
		//eoc_log("(%s): wrong \"line_probe\" value in %s profile: %d , may be 0,1",config_file,name,line_probe);
		printf("(%s): wrong \"line_probe\" value in %s profile: %d , may be 0,1\n",config_file,name,line_probe);
		return -1;
	    }

	    conf_profile *nprof = new conf_profile;
	    nprof->name = name;
	    nprof->nsize = strlen(name);

	    nprof->conf.annex = (annex_t)annex;
	    nprof->conf.wires = (wires_t)wires;
	    nprof->conf.power = (power_t)power;
// ????	    nprof->conf.psd = 0;
	    nprof->conf.clk = (clk_t)ref_clock;
	    nprof->conf.line_probe = (line_probe_t)line_probe;
// ????	    nprof->conf.remote_cfg = (remote_cfg_t)rem_cfg; 
	    nprof->conf.min_rate = max_rate;
	    nprof->conf.max_rate = min_rate;

	    nprof->conf.use_cur_down = use_cur_down;
	    nprof->conf.use_worst_down = use_worst_down;
	    nprof->conf.use_cur_up = use_cur_up;
	    nprof->conf.use_worst_up = use_worst_up;
	    
	    nprof->conf.cur_marg_down = cur_marg_down;
	    nprof->conf.worst_marg_down = worst_marg_down;
	    nprof->conf.cur_marg_up = cur_marg_up;
	    nprof->conf.worst_marg_up = worst_marg_up;
	    conf_profs.add(nprof);

	}catch(ConfigException& cex){
    	    if( name ){
		// eoc_log("Error while parsing profile: %s",name);
		printf("(%s): error while parsing profile: %s\n",config_file,name);
		return -1;
	    }
	    break;
	}catch(...){
	    cout << "fail get element" << endl;
	}
    }
/*
    //----------- read span alarm profiles ------------------//
    // 1. Check that span_profile group exist
    try{
	Setting &s = cfg.lookup("span_profiles");
    }catch(...){
        // eoc_log("Cannot found \"span_profiles\" section in %s file",file);
        printf("Cannot found \"span_profiles\" section in %s file\n",file);
        return -1;
    }

    // 2. read all elements of span_profiles
    while(1){
	name = NULL;
	try{
    	    Setting &s = cfg.lookup("span_profiles");
	    const char *str = s[i]["name"];
	    if( !(name = strndup(str,SNMP_ADMIN_LEN)) ){
		//eoc_log("Not enougth memory");
		printf("Not enougth memory\n");
		return -1;
	    }
	    int wires = s[i]["wires"];
	    int min_rate = s[i]["min_rate"];
	    int max_rate = s[i]["max_rate"];
	    int annex = s[i]["annex"];
	    int power = s[i]["power"];
	    int ref_clock = s[i]["ref_clock"];
	    int line_probe = s[i]["line_probe"];
	    
	    // Check input values
	    if( wires > 4 || wires <= 0 ){
		//eoc_log("Wrong \"wires\" value in %s profile: %d , may be 1-4",name,wires);
		printf("Wrong \"wires\" value in %s profile: %d , may be 1-4\n",name,wires);
		return -1;
	    }
	    if( annex > 3 || annex < 0){
		//eoc_log("Wrong \"annex\" value in %s profile: %d , may be 0-3",name,annex);
		printf("Wrong \"wires\" value in %s profile: %d , may be 1-4\n",name,annex);
		return -1;
	    }
	    	
	    if( power != 1 && power != 0){
		//eoc_log("Wrong \"power\" value in %s profile: %d , may be 0,1",name,power);
		printf("Wrong \"power\" value in %s profile: %d , may be 0,1\n",name,power);
		return -1;
	    }

	    if( ref_clock > 3 || ref_clock < 0){
		//eoc_log("Wrong \"ref_clock\" value in %s profile: %d , may be 0-3",name,ref_clock);
		printf("Wrong \"ref_clock\" value in %s profile: %d , may be 1-4\n",name,ref_clock);
		return -1;
	    }
	    	
	    if( line_probe != 1 && line_probe != 0){
		//eoc_log("Wrong \"line_probe\" value in %s profile: %d , may be 0,1",name,line_probe);
		printf("Wrong \"line_probe\" value in %s profile: %d , may be 0,1\n",name,line_probe);
		return -1;
	    }

	    span_profile *nprof = new span_profile;
	    nprof->name = name;
	    nprof->nsize = strlen(name);
	    nprof->wires = wires;
	    nprof->annex = annex;
	    nprof->power = power;
	    nprof->ref_clk = ref_clock;
	    nprof->line_probe = line_probe;
	    nprof->min_rate = min_rate;
	    nprof->max_rate = max_rate;
	    conf_profs.add(nprof);
	    cout << "ADD: " << name << " " << wires << " " << min_rate << " " << max_rate << " " << annex << " " << power << " " << ref_clock << " " << line_probe << endl;
	}catch(ConfigException& cex){
    	    if( name ){
		// eoc_log("Error while parsing profile: %s",name);
		printf("Error while parsing profile: %s\n",name);
		return -1;
	    }
	    break;
	}catch(...){
	    cout << "fail get element" << endl;
	}
    }
*/

    //----------- read channels configuration ------------------//
    // 1. Check that channels group exist
    try{
	Setting &s = cfg.lookup("channels");
    }catch(...){
        // eoc_log("(%s): cannot found \"channels\" section",config_file);
        printf("(%s): cannot found \"channels\" section\n",config_file);
        return -1;
    }

    // 2. read all elements of channels
    for(i=0;1;i++){
	name = NULL;
	try{
    	    Setting &s = cfg.lookup("channels");
	    const char *str = s[i]["name"];
	    if( !(name = strndup(str,SNMP_ADMIN_LEN)) ){
		//eoc_log("Not enougth memory");
		printf("Not enougth memory\n");
		return -1;
	    }
	    int master = s[i]["master"];
	    
	    if( master != 1 && master != 0){
		//eoc_log("(%s): wrong \"master\" value in %s channel: %d , may be 0,1",config_file,name,master);
		printf("(%s): wrong \"master\" value in %s channel: %d , may be 0,1\n",config_file,name,master);
		return -1;
	    }

	    // If channel is slave - only responder part
	    if( !master ){
		if( add_slave(name) ){
		    //eoc_log("(%s): cannot add channel \"%s\" - no such device",config_file,name);
		    printf("(%s): cannot add channel \"%s\" - no such device\n",config_file,name);
		}    
		continue;
	    }
	    
	    char *cprof = strndup(s[i]["conf_profile"],SNMP_ADMIN_LEN);
	    if( !cprof ){
		//eoc_log("(%s): Not enought memory",config_file);
		printf("(%s): Not enought memory\n",config_file);
		return -1;
	    }
	    if( !conf_profs.find((char*)cprof,strlen(str)) ){
		//eoc_log("(%s): wrong \"conf_profile\" value in %s channel: %s, no such profile",config_file,name,cprof);
		printf("(%s) wrong \"conf_profile\" value in %s channel: %s, no such profile\n",config_file,name,cprof);
		return -1;
	    }
/*
	    char *aprof = strndup(s[i]["alarm_profile"],SNMP_ADMIN_LEN);
	    if( !aprof ){
		//eoc_log("(%s): Not enought memory",config_file);
		printf("(%s): Not enought memory\n",config_file);
		return -1;
	    }
	    if( !alarm_profs.find((char*)str,strlen(str)) ){
		//eoc_log("(%s): wrong \"alarm_profile\" value in %s channel: %s, no such profile",config_file,name,aprof);
		printf("(%s) wrong \"alarm_profile\" value in %s channel: %s, no such profile\n",config_file,name,aprof);
		return -1;
	    }

*/	    
	    
	    int repeaters = s[i]["repeaters"];
	    if( repeaters <0 || repeaters > MAX_REPEATERS ){
		//eoc_log("(%s): wrong \"repeaters\" value in %s channel: %d, may be 1-%d",config_file,name,repeaters,MAX_REPEATERS);
		printf("(%s): wrong \"conf_profile\" value in %s channel: %d, may be 1-%d\n",config_file,name,repeaters,MAX_REPEATERS);
		return -1;
	    }
	    
	    // TODO: Add alarm handling
	    if( add_master(name,cprof,NULL,repeaters,tick_per_min) ){
		//eoc_log("(%s): cannot add channel \"%s\" - no such device",config_file,name);
		printf("(%s): cannot add channel \"%s\" - no such device\n",config_file,name);
		continue;
	    }
	    cout << name << " " << master << " " << str << endl;
	}catch(ConfigException& cex){
    	    if( name ){
		// eoc_log("(%s): error while parsing profile: %s",config_file,name);
		printf("(%s): error while parsing profile: %s\n",config_file,name);
		return -1;
	    }
	    break;
	}catch(...){
	    cout << "fail get element" << endl;
	}
    }

    return 0;
}

int EOC_main::
add_slave(char *name)
{
    EOC_dev_terminal *dev = init_dev(name);
    if( !dev )
	return -1;
    channel_elem *el = new channel_elem(dev);
    el->name = name;
    el->nsize = strlen(name);
    channels.add(el);
    return 0;
}

int EOC_main::
add_master(char *name,char *cprof, char *aprof,int reps,int tick_per_min)
{
    EOC_dev_terminal *dev = (EOC_dev_terminal *)init_dev(name);
    if( !dev )
	return -1;
	
    EOC_config *cfg = new EOC_config(&conf_profs,&alarm_profs,cprof,aprof,reps);
    channel_elem *el = new channel_elem(dev,cfg,tick_per_min);
    el->name = name;
    el->nsize = strlen(name);
    channels.add(el);
    return 0;
}

int EOC_main::
configure_channels()
{
    channel_elem *el = (channel_elem*)channels.first();
    while( el ){
	el->eng->configure(el->name);
	el = (channel_elem*)channels.next(el->name,el->nsize);
    }
}


int EOC_main::
poll_channels()
{
    channel_elem *el = (channel_elem*)channels.first();
    while( el ){
	el->eng->schedule();
	el = (channel_elem*)channels.next(el->name,el->nsize);
    }
}

// ------------ Application requests ------------------------------//

void EOC_main::
app_listen()
{
    char *buff;
    int size,conn;
    int ret;

    if( !app_srv.wait() )
	return;
	
//    while( (size = app_srv.recv(conn,buff) ) ){
    while( 1 ){
	size = app_srv.recv(conn,buff);
	if( !size )
	    return;
	app_frame fr(buff,size);
	if( !fr.frame_ptr() ){
	    delete buff;
	    continue; 
	}
	if( ret = app_request(&fr) )
	    continue;
	app_srv.send(conn,fr.frame_ptr(),fr.frame_size());
    }
}


int EOC_main::
app_request(app_frame *fr)
{
    if( fr->role() != app_frame::REQUEST )
	return -1;
    
    printf("App request, ID = %d\n",fr->id());
    switch( fr->id() ){
    case app_frame::SPAN_CONF_PROF:
	return app_spanconf_prof(fr);
    case app_frame::ENDP_ALARM_PROF:
	return app_spanconf_prof(fr);
    }

    if( fr->chan_name() )
	return app_chann_request(fr);
    return -1;
}

int EOC_main::
app_chann_request(app_frame *fr)
{
    // check that requested channel exist
    channel_elem *el = (channel_elem *)
	    channels.find((char*)fr->chan_name(),strlen(fr->chan_name()));
    if( !el ) // No such channel on this device
	return -1;
    EOC_engine *eng = el->eng;
    if( eng->get_type() != master ) // Channel do not maintain EOC DB
	return -1;
    EOC_engine_act *eng_a = (EOC_engine_act *)eng;
    return eng_a->app_request(fr);
}

int EOC_main::
app_spanconf_prof(app_frame *fr)
{
    span_conf_prof_payload *p = (span_conf_prof_payload*)fr->payload_ptr();
    int len = strnlen(p->ProfileName,SNMP_ADMIN_LEN+1);

    conf_profile *prof;
    switch(fr->type()){
    case app_frame::GET:
	if( !len ){
	    fr->negative();
	    return 0;
	}
	prof = (conf_profile *)conf_profs.find(p->ProfileName,len);
	if( !prof ){ // No such profile
	    fr->negative();
	    return 0;
	}
	fr->response();
	p->conf = prof->conf;
	return 0;
    case app_frame::GET_NEXT:
        if( !len ){ // requested first entry 
	    prof = (conf_profile *)conf_profs.first();
	}else{
	    prof = (conf_profile *)conf_profs.next(p->ProfileName,len);
	}
	if( !prof ){
	    fr->negative();
	    return 0;
	}
	fr->response();
	p->conf = prof->conf;
	return 0;
    case app_frame::SET:

/*
	1. Узнать есть ли уже этот профиль
	2. Если нету и не стоит пометка создать имя профиля - сброс
	3. Если есть - внести изменения
	4. Все интерфейсы имеющие этот профиль перенастроить
*/
	return 0;
    }
    
}     

int EOC_main::
app_endpalarm_prof(app_frame *fr)
{
/*
    endp_alarm_prof_payload *p = (endp_alarm_prof_payload*)fr->payload_ptr();
    int len = strnlen(p->ProfileName,SNMP_ADMIN_LEN+1);
    if( !len )
    alarm_profile *prof;
    switch(fr->type()){
    case app_frame::GET:
	prof = (alarm_profile *)alarm_profs.find(p->ProfileName,len);
	if( !prof ){ // No such profile
	    fr->negative();
	    return 0;
	}
	fr->response();
	p->alarm = prof->alarm;
	return 0;
    case app_frame::GET_NEXT:
        if( !len ){ // requested first entry 
	    prof = (alarm_profile *)alarm_profs.first();
	}else{
	    prof = (alarm_profile *)alarm_profs.next(p->ProfileName,len);
	}
	if( !prof ){
	    fr->negative();
	    return 0;
	}
	fr->response();
	p->alarm = prof->alarm;
	return 0;
    case app_frame::SET:
	return 0;
/*	1. Узнать есть ли уже этот профиль
	2. Если нету и не стоит пометка создать имя профиля - сброс
	3. Если есть - внести изменения
	4. Все интерфейсы имеющие этот профиль перенастроить
    }
*/
    return 0;    
}

