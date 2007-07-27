#include <iostream>
#include <libconfig.h++>
#include <snmp/snmp-generic.h>
#include <utils/hash_table.h>
#include <config/EOC_config.h>

#include <devs/EOC_dev.h>
#include <devs/EOC_dev_master.h>

#include <EOC_main.h>
#include <span_profile.h>
#include <channel.h>

using namespace libconfig;
using namespace std;

EOC_dev_master *init_dev(char *name);

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
    // 1. Check that span_profile group exist
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
    // 1. Check that span_profile group exist
    try{
	Setting &s = cfg.lookup("span_profiles");
    }catch(...){
        // eoc_log("(%s): cannot found \"span_profiles\" section",config_file);
        printf("(%s): cannot found \"span_profiles\" section\n",config_file);
        return -1;
    }

    // 2. read all elements of span_profiles
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
	    int min_rate = s[i]["min_rate"];
	    int max_rate = s[i]["max_rate"];
	    int annex = s[i]["annex"];
	    int power = s[i]["power"];
	    int ref_clock = s[i]["ref_clock"];
	    int line_probe = s[i]["line_probe"];
	    
	    // Check input values
	    if( wires > 4 || wires <= 0 ){
		//eoc_log("(%s): wrong \"wires\" value in %s profile: %d , may be 1-4",config_file,name,wires);
		printf("(%s): wrong \"wires\" value in %s profile: %d , may be 1-4\n",config_file,name,wires);
		return -1;
	    }
	    if( annex > 3 || annex < 0){
		//eoc_log("(%s): wrong \"annex\" value in %s profile: %d , may be 0-3",config_file,name,annex);
		printf("(%s): wrong \"wires\" value in %s profile: %d , may be 1-4\n",config_file,name,annex);
		return -1;
	    }
	    	
	    if( power != 1 && power != 0){
		//eoc_log("(%s): wrong \"power\" value in %s profile: %d , may be 0,1",config_file,name,power);
		printf("(%s) wrong \"power\" value in %s profile: %d , may be 0,1\n",config_file,name,power);
		return -1;
	    }

	    if( ref_clock > 3 || ref_clock < 0){
		//eoc_log("(%s): wrong \"ref_clock\" value in %s profile: %d , may be 0-3",config_file,name,ref_clock);
		printf("(%s): wrong \"ref_clock\" value in %s profile: %d , may be 1-4\n",config_file,name,ref_clock);
		return -1;
	    }
	    	
	    if( line_probe != 1 && line_probe != 0){
		//eoc_log("(%s): wrong \"line_probe\" value in %s profile: %d , may be 0,1",config_file,name,line_probe);
		printf("(%s): wrong \"line_probe\" value in %s profile: %d , may be 0,1\n",config_file,name,line_probe);
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
    EOC_dev *dev = init_dev(name);
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
    EOC_dev_master *dev = (EOC_dev_master *)init_dev(name);
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
poll_channels()
{
    channel_elem *el;
    channels.init_trace();
    while( ( el = (channel_elem*)channels.next_elem()) ){
	el->eng->schedule();
    }    
}

