// system includes
#include <sys/types.h>
#include <syslog.h>
#include <dirent.h>

// lib includes
#include <iostream>
#include <libconfig.h++>

// local includes

#define EOC_DEBUG
#include <generic/EOC_generic.h>
#include <snmp/snmp-generic.h>
#include <utils/hash_table.h>
#include <config/EOC_config.h>

#include <devs/EOC_dev.h>
#include <devs/EOC_dev_terminal.h>
#include <devs/EOC_mr17h.h>
#include <devs/EOC_mr16h.h>

#include <EOC_main.h>
#include <EOC_pci.h>
#include <EOC_cfg_options.h>

#include <app-if/err_codes.h>

#define CONF_FILE_NAME "/etc/eocd/eocd.conf"

using namespace libconfig;
using namespace std;
//---------------------------------------------------------------------//
EOC_dev_terminal *init_dev(char *name);
void delete_dev(EOC_dev *dev);

static void del_channel_elem(hash_elem *h) {
	delete (channel_elem*) h;
}
static void del_conf_profile(hash_elem *h) {
	delete (conf_profile*) h;
}

int main() {
	debug_lev = DFULL;
	// 3. Channels creation/deleteion
	hash_table conf_profs(SNMP_ADMIN_LEN), channels(IF_NAME_LEN);
	for (int i = 0; i < 200; i++) {
		char pname[256];
		sprintf(pname, "def%d", i);
		conf_profile *cprof = new conf_profile;
		memset(cprof, 0, sizeof(conf_profile));
		cprof->name = strdup(pname);
		cprof->nsize = strlen(cprof->name);
		cprof->access = conf_profile::profRW;
		// Profile settings
		cprof->conf.annex = annex_a;
		cprof->conf.wires = twoWire;
		cprof->conf.power = noPower;
		cprof->conf.clk = localClk;
		cprof->conf.line_probe = disable;
		cprof->conf.rate = 2304;
		cprof->conf.tcpam = tcpam16;
		// compatibility: default is basic
		cprof->comp = EOC_dev::comp_base;
		conf_profs.add(cprof);
	}
	while (1) {
		for (int i = 2; i < 4; i++) {
			char name1[64],*name;
			char *cprof = strdup("def10");
			sprintf(name1, "dsl%d", i);
			name = strdup(name1);
			PDEBUG(DFULL, "Initialize device");
			EOC_dev_terminal *dev = init_dev(name);
			if (!dev) {
				PDEBUG(DERR, "dev = %p", dev);
				return -1;
			}
			PDEBUG(DFULL, "Initialize config");
			EOC_config *cfg = new EOC_config(&conf_profs, cprof, 1);
			PDEBUG(DERR, "CONFIG=%p,cprof=%s", cfg, cfg->cprof());
			channel_elem *el = new channel_elem(dev, cfg,delete_dev);
			el->name = name;
			el->nsize = strlen(name);
			//el->is_updated = 1;
			PDEBUG(DFULL, "%s pointer = %p", el->name, el);
			channels.add(el);
			channels.sort();
		}
		channels.clear(del_channel_elem);
	}


	/*
	 // 2.(OK) Conf profiles table management
	 hash_table conf_profs(SNMP_ADMIN_LEN);
	 while (1) {
	 printf("\n-----------------ADD--------------------\n");
	 for (int i = 0; i < 2000; i++) {
	 char pname[256];
	 sprintf(pname, "def%d", i);
	 conf_profile *cprof = new conf_profile;
	 memset(cprof, 0, sizeof(conf_profile));
	 cprof->name = strdup(pname);
	 cprof->nsize = strlen(cprof->name);
	 cprof->access = conf_profile::profRW;
	 // Profile settings
	 cprof->conf.annex = annex_a;
	 cprof->conf.wires = twoWire;
	 cprof->conf.power = noPower;
	 cprof->conf.clk = localClk;
	 cprof->conf.line_probe = disable;
	 cprof->conf.rate = 2304;
	 cprof->conf.tcpam = tcpam16;
	 // compatibility: default is basic
	 cprof->comp = EOC_dev::comp_base;
	 conf_profs.add(cprof);
	 }
	 printf("\n-----------------CLEAR------------------\n");
	 sleep(1);
	 conf_profs.clear(del_conf_profile);
	 }
	 /*
	 // 1. (OK) Test profile createion/destruction
	 while (1) {
	 char *defname = "default";
	 conf_profile *cprof = new conf_profile;
	 memset(cprof, 0, sizeof(conf_profile));
	 cprof->name = strdup("default");
	 cprof->nsize = strlen(cprof->name);
	 cprof->access = conf_profile::profRO;
	 // Profile settings
	 cprof->conf.annex = annex_a;
	 cprof->conf.wires = twoWire;
	 cprof->conf.power = noPower;
	 cprof->conf.clk = localClk;
	 cprof->conf.line_probe = disable;
	 cprof->conf.rate = 2304;
	 cprof->conf.tcpam = tcpam16;
	 // compatibility: default is basic
	 cprof->comp = EOC_dev::comp_base;
	 // Check compatibility. Should never be erroneous
	 if (cprof->check_comp()) {
	 syslog(
	 LOG_ERR,"eocd(read_config): Fatal error: \"default\" profile is not compatible with basical level");
	 PDEBUG(
	 DERR,
	 "eocd(read_config): Fatal error: \"default\" profile is not compatible with basical level");
	 exit(1);
	 }
	 delete cprof;
	 }
	 */
}

/*

 int EOC_main::
 read_config()
 {
 char *name;
 int i;
 tick_per_min = TICK_PER_MINUTE_DEF;
 hash_elem *el;
 char *opt;
 char *endp;
 unsigned long tmp_val;
 int ret;
 int retval = 0;

 PDEBUG(DFULL,"Start");
 // Free KDB
 kdbinit();

 //----------- read tick per minute value ------------------//
 // 1. Check that conf_profile group exist
 if( ret = kdb_appget("sys_eocd_actpermin",&opt) ) {
 if( ret> 1 ) {
 syslog(LOG_ERR,"eocd(read_config): Found several options containing \"sys_eocd_actpermin\". Use first: %s",opt);
 PDEBUG(DERR,"eocd(read_config): Found several options containing \"sys_eocd_actpermin\". Use first: %s",opt);
 }
 tmp_val = strtoul(opt,&endp,0);
 if( opt != endp )
 tick_per_min = tmp_val;
 }
 PDEBUG(DINFO,"tick_p_min = %d",tick_per_min);

 //----------- read span configuration profiles ------------------//
 do {
 char *saveptr, *token = NULL;
 char *prof_prefix = "sys_eocd_sprof";
 char *popt;
 char *topt = opt;

 // 1. Add default configuration profile compatible with all devices
 {
 }

 // 2. read all elements of conf_profiles
 #define OPTINDEX(grp,grpsize,name,idx) \
	idx = -1; \
	for(int k=0;k<grpsize;k++){ \
		if( !strncmp(grp[k].optname(),name,MAX_OPTSTR) ){ \
			idx = k; \
			break; \
		} \
	}

 // Changing profile index till fault
 for(int i=0;;i++) {
 char option[256];
 cfg_option prof_opts[] = {cfg_option(1,"name"),cfg_option(1,"tcpam",tcpam_vals,tcpam_vsize),
 cfg_option(1,"annex",annex_vals,annex_vsize),cfg_option(1,"power",power_vals,power_vsize),
 cfg_option(1,"rate",0,0,1,64),cfg_option(0,"mrate",0,0,1,64),cfg_option(1,"comp",comp_vals,comp_vsize)};
 int prof_optsnum = sizeof(prof_opts)/sizeof(cfg_option);

 char *str1,*str2;
 char *token,*subtoken;
 char *saveptr1,*saveptr2;
 int j;

 sprintf(option,"sys_eocd_sprof_%d",i);
 if( !(ret = kdb_appget(option,&opt)) ) {
 break;
 } else if( ret> 1 ) {
 syslog(LOG_ERR,"eocd(read_config): Found several options containing \"%s\". Use first: %s",option,opt);
 PDEBUG(DERR,"eocd(read_config): Found several options containing \"%s\". Use first: %s",option,opt);
 }

 str1 = opt;

 for (j = 1;; j++, str1 = NULL) {
 token = strtok_r(str1, " ", &saveptr1);
 if (token == NULL)
 break;
 printf("%d: %s\n", j, token);

 char ptrs[2][MAX_OPTSTR];
 int i;
 for (i=0,str2 = token; i<2; str2 = NULL,i++) {
 subtoken = strtok_r(str2,"=", &saveptr2);
 if (subtoken == NULL)
 break;
 //printf(" --> %s\n", subtoken);
 strncpy(ptrs[i],subtoken,MAX_OPTSTR);
 }
 printf("%s = %s\n",ptrs[0],ptrs[1]);
 for(i=0;i<prof_optsnum;i++) {
 cfg_option::chk_res ret;
 ret = prof_opts[i].chk_option(ptrs[0],ptrs[1]);
 if( ret == cfg_option::Accept || ret == cfg_option::Exist )
 break;
 }
 }

 int errflg = 0;
 for(j=0;j<prof_optsnum;j++) {
 if( !prof_opts[j].is_valid() && prof_opts[j].is_basic() ) {
 syslog(LOG_ERR,"eocd(read_config): profile \"%s\", wrong \"%s\" value",option,prof_opts[j].optname());
 PDEBUG(DERR,"eocd(read_config): profile \"%s\", wrong \"%s\" value",option,prof_opts[j].optname());
 errflg = 1;
 }
 if( !strncmp(prof_opts[j].optname(),"name",5) && prof_opts[j].is_int() ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR \"%s\" should be string value",prof_opts[j].optname());
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR \"%s\" should be string value",prof_opts[j].optname());
 exit(0);
 } else if( strncmp(prof_opts[j].optname(),"name",5) && !prof_opts[j].is_int() ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR \"%s\" should be int value",prof_opts[j].optname());
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR \"%s\" should be int value",prof_opts[j].optname());
 exit(0);
 }
 // DEBUG
 if( prof_opts[j].is_int() ) {
 PDEBUG(DFULL," %s = %d",prof_opts[j].optname(),prof_opts[j].value());
 } else {
 PDEBUG(DFULL," %s = %s",prof_opts[j].optname(),prof_opts[j].svalue());
 }
 }

 if( errflg ) { // Skip prifile
 syslog(LOG_ERR,"eocd(read_config): Skip profile \"%s\"",option);
 PDEBUG(DERR,"eocd(read_config): Skip profile \"%s\"",option);
 continue;
 }

 // Get profile name
 int ind;
 char *curopt;
 // Profile name
 curopt = "name";
 OPTINDEX(prof_opts,prof_optsnum,curopt,ind);
 if( ind < 0 ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 exit(0);
 }
 // Find profile
 int namelen = strnlen(prof_opts[ind].svalue(),256);
 conf_profile *cprof = (conf_profile *)conf_profs.find(prof_opts[ind].svalue(),namelen);
 if( !cprof ) {
 cprof = new conf_profile;
 memset(cprof,0,sizeof(conf_profile));
 cprof->name = strndup(prof_opts[ind].svalue(),256);
 cprof->nsize = namelen;
 // Profile service info
 cprof->access = conf_profile::profRW;
 // Profile content
 cprof->conf.wires = twoWire; // nprof->conf.wires = (wires_t)wires;
 cprof->conf.line_probe = disable;
 conf_profs.add(cprof);
 } else if( cprof->access == conf_profile::profRO ) {
 continue;
 }

 // Profile annex value
 curopt = "annex";
 OPTINDEX(prof_opts,prof_optsnum,curopt,ind);
 if( ind < 0 ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 exit(0);
 }
 cprof->conf.annex = (annex_t)prof_opts[ind].value();

 // Profile power value
 curopt = "power";
 OPTINDEX(prof_opts,prof_optsnum,curopt,ind);
 if( ind < 0 ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 exit(0);
 }
 cprof->conf.power = (power_t)prof_opts[ind].value();

 // Profile tcpam val
 curopt = "tcpam";
 OPTINDEX(prof_opts,prof_optsnum,curopt,ind);
 if( ind < 0 ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 exit(0);
 }
 cprof->conf.tcpam = (tcpam_t)prof_opts[ind].value();

 // Profile tcpam val
 curopt = "rate";
 OPTINDEX(prof_opts,prof_optsnum,curopt,ind);
 if( ind < 0 ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 exit(0);
 }
 cprof->conf.rate = prof_opts[ind].value();
 if( (int)cprof->conf.rate < 0 ) {
 // Get mrate value
 PDEBUG(DFULL,"Get mrate option value");
 curopt = "mrate";
 OPTINDEX(prof_opts,prof_optsnum,curopt,ind);
 if( ind < 0 ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 exit(0);
 }
 cprof->conf.rate = prof_opts[ind].value();
 }

 // compatibility
 curopt = "comp";
 OPTINDEX(prof_opts,prof_optsnum,curopt,ind);
 if( ind < 0 ) {
 syslog(LOG_ERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 PDEBUG(DERR,"eocd(read_config): FATAL ERROR cannot find \"%s\" index",curopt);
 exit(0);
 }
 cprof->comp = (EOC_dev::compatibility_t)prof_opts[ind].value();

 // Check compatibility
 if( cprof->check_comp() ) {
 PDEBUG(DERR,"Not compatible!");
 char buf[256];
 EOC_dev::comp_name(cprof->comp,buf);
 syslog(LOG_ERR,"eocd(read_config): Skip profile \"%s\", settings not compatible with %s level",
 cprof->name,buf);
 PDEBUG(DERR,"After syslog!");
 PDEBUG(DERR,"eocd(read_config): Skip profile \"%s\", settings not compatible with %s level",
 cprof->name,buf);
 continue;
 }

 PDEBUG(DERR,"Save profile %s",cprof->name);
 cprof->is_updated = 1;
 }
 }while(0);
 PDEBUG(DFULL,"Clear conf table from old profiles");
 conf_profs.clear(del_conf_profile);
 conf_profs.sort();

 //----------- read channels configuration ------------------//
 do {
 char *saveptr = NULL;
 char *chan_prefix = "sys_eocd_chan";
 char *popt;
 char *topt;
 char chlist[512];

 if( !(ret = kdb_appget("sys_eocd_channels",&opt) ) ) {
 break;
 } else if( ret> 1 ) {
 syslog(LOG_ERR,"eocd(read_config): Found several options containing \"sys_eocd_channels\". Use first: %s",opt);
 PDEBUG(DERR,"eocd(read_config): Found several options containing \"sys_eocd_channels\". Use first: %s",opt);
 }
 strncpy(chlist,opt,512);

 PDEBUG(DFULL,"start channels processing, opt=\"%s\"",chlist);
 for(topt=chlist;;topt=NULL) {
 char pci_pair[256],*ptoken,*tpair=NULL;
 char *saveptr1 = NULL;
 int pcislot = -1;
 int pcidev = -1;
 int valid = 1;
 char option[256];
 char *option_val;

 if( !(ptoken = strtok_r(topt," ",&saveptr)) ) {
 PDEBUG(DFULL,"ptoken = 0");
 break;
 }

 strncpy(pci_pair,ptoken,256);
 PDEBUG(DFULL,"Process channel: pci_pair(%p)=%s",pci_pair,pci_pair);

 int cnt = 0;
 for(tpair=pci_pair;;tpair=NULL,cnt++) {
 char *token = strtok_r(tpair,":",&saveptr1);
 PDEBUG(DFULL,"pci_pair token=%s",token);
 if( !token )
 break;
 tmp_val = strtoul(token,&endp,0);
 if( token == endp ) {
 break;
 }
 switch(cnt) {
 case 0:
 pcislot = tmp_val;
 break;
 case 1:
 pcidev = tmp_val;
 break;
 }
 }

 if( cnt != 2 ) {
 syslog(LOG_ERR,"eocd(read_config): Error channel identification: %s",pci_pair);
 PDEBUG(DERR,"eocd(read_config): Error channel identification: %s, slot=%d, dev=%d",pci_pair,pcislot,pcidev);
 continue;
 }
 name = pci2dname(pcislot,pcidev);
 if( !name ) {
 syslog(LOG_ERR,"eocd(read_config): Error channel identification: %s, no correspond iface",pci_pair);
 PDEBUG(DERR,"eocd(read_config): Error channel identification: %s, no correspond iface",pci_pair);
 continue;
 }
 PDEBUG(DINFO,"pci2dname=%s",name);

 // Master/Slave option. default = slave
 int master = 0;
 snprintf(option,256,"%s_s%04d_%d_master",chan_prefix,pcislot,pcidev);
 PDEBUG(DFULL,"master option=%s",option);
 if( ret = kdb_appget(option,&option_val)) {
 master = atoi(option_val);
 } else {
 master = -1;
 }

 if( master != 1 && master != 0) {
 syslog(LOG_ERR,"eocd(read_config): wrong \"%s\" value in %s channel: %d , may be 0,1. Will set default",
 option,name,master);
 PDEBUG(DERR,"eocd(read_config): wrong \"%s\" value in %s channel: %d , may be 0,1. Will set default",
 option,name,master);
 master = -1;
 }

 // Configuration profile option
 // if configuration profile not exist or option is missing set cprof to "default"
 snprintf(option,256,"%s_s%04d_%d_confprof",chan_prefix,pcislot,pcidev);
 char *cprof;
 if( ret = kdb_appget(option,&option_val)) {
 cprof = strndup(option_val,SNMP_ADMIN_LEN);
 } else {
 cprof = strdup("default");
 }

 PDEBUG(DFULL,"CPROF=%s",cprof);
 // check that strdup succeed
 if( !cprof ) {
 syslog(LOG_ERR,"eocd(read_config): Not enought mmory");
 PDEBUG(DERR,"eocd(read_config): Not enought memory");
 exit(1);
 }

 PDEBUG(DFULL,"Check CPROF=%s",cprof);
 // check that profile exist
 if( conf_profs.find((char*)cprof,strlen(cprof)) == NULL ) {
 PDEBUG(DERR,"Error in find cprof");
 syslog(LOG_ERR,"eocd(read_config): wrong \"%s\" value in %s channel: no such profile \"%s\", set \"default\"",
 option,name,cprof);
 PDEBUG(DERR,"eocd(read_config): wrong \"%s\" value in %s channel: no such profile \"%s\", set \"default\"",
 option,name,cprof);
 free(cprof);
 cprof = strdup("default");
 }
 PDEBUG(DFULL,"CPROF=%s is exist",cprof);

 // Apply configuration to channel. We now can always apply
 int eocd_apply = 1;


 // Check existence of channel
 channel_elem *el = (channel_elem *)channels.find(name,strlen(name));
 PDEBUG(DFULL,"el=%p",el);
 if( master < 0 ) {
 if( !el ) {
 // if master option is not specified and channel not exist
 // setup it as slave
 master = 0;
 } else {
 if( el->eng->get_type() == slave ) {
 master = 0;
 } else {
 master = 1;
 }
 }
 }
 // If channel is slave OR
 // master option is broken but channel exist & it is slave
 // setup only responder part
 if( !master ) {
 PDEBUG(DINFO,"Add slave");
 if( name ) {
 if( add_slave(name,cprof) ) {
 syslog(LOG_ERR,"eocd(read_config): cannot add channel \"%s\" - no such device",name);
 PDEBUG(DERR,"eocd(read_config): cannot add channel \"%s\" - no such device",name);
 }
 }
 continue;
 }

 // PBO options
 int pbomode = 0;
 snprintf(option,256,"%s_s%04d_%d_pbomode",chan_prefix,pcislot,pcidev);
 if( ret = kdb_appget(option,&option_val)) {
 pbomode = atoi(option_val);
 }

 snprintf(option,256,"%s_s%04d_%d_pboval",chan_prefix,pcislot,pcidev);
 char *pboval;
 if( ret = kdb_appget(option,&option_val)) {
 pboval = strndup(option_val,128);
 } else {
 pboval = strdup("");
 }
 // check that strdup succeed
 if( !cprof ) {
 syslog(LOG_ERR,"eocd(read_config): Not enought mmory");
 PDEBUG(DERR,"eocd(read_config): Not enought memory");
 continue;
 }

 // Repeaters option
 int repeaters = 0;
 snprintf(option,256,"%s_s%04d_%d_regs",chan_prefix,pcislot,pcidev);
 if( ret = kdb_appget(option,&option_val)) {
 repeaters = atoi(option_val);
 }
 if( repeaters <0 || repeaters> MAX_REPEATERS ) {
 syslog(LOG_ERR,"eocd(read_config): wrong \"%s\" value in %s channel: %d , may be 0-%d",
 option,name,repeaters,MAX_REPEATERS);
 PDEBUG(DERR,"eocd(read_config): wrong \"%s\" value in %s channel: %d , may be 0-%d",
 option,name,repeaters,MAX_REPEATERS);
 continue;
 }

 PDEBUG(DINFO,"%s: apply config from cfg-file = %d",name,eocd_apply);
 // TODO: Add alarm handling
 PDEBUG(DERR,"Add master");
 if( name ) {
 PDEBUG(DERR,"Call add_master");
 if( add_master(name,cprof,NULL,repeaters,tick_per_min,pbomode,pboval,eocd_apply) ) {
 syslog(LOG_ERR,"(%s): cannot add channel \"%s\" - no such device",
 config_file,name);
 PDEBUG(DERR,"(%s): cannot add channel \"%s\" - no such device",
 config_file,name);
 free(pboval);
 continue;
 }
 }
 free(pboval);
 }
 }while(0);
 PDEBUG(DFULL,"Clear Channels table from old channels");
 channels.clear(del_channel_elem);
 channels.sort();

 // Close KDB data base
 db_close();
 exit:
 PDEBUG(DFULL,"retval=%d",retval);
 return retval;
 }

 // Write command is not implemented in this variant of configuration
 int EOC_main::
 write_config() {return -ENIMPL;}

 #endif // KDB_CONFIG


 int EOC_main::add_slave(char *name, char *cprof, int app_cfg) {
 PDEBUG(DERR, "Add slave %s", name);

 do {
 channel_elem *el = (channel_elem *) channels.find(name, strlen(name));
 if (el) { // If this device exist in current configuration

 if (el->eng->get_type() != slave) {
 PDEBUG(DERR, "Reinit device %s", name);
 PDEBUG(DFULL,"%s pointer = %p",el->name,el);
 channel_elem *d = (channel_elem*) channels.del(name, strlen(
 name));
 delete d;
 break;
 }
 // Check if some of optins changed
 EOC_config *cfg = ((EOC_engine_act*) el->eng)->config();
 // Configuration profile name
 cfg->cprof(cprof);
 if (el->eng->check_compat()) {
 char *p = strdup("default");
 cfg->cprof(p);
 syslog(
 LOG_ERR,
 "eocd(add_master): Device \"%s\": not compatible with profile \"%s\", set profile=\"default\"",
 el->eng->dev1_name(), cprof);
 PDEBUG(
 DERR,
 "eocd(add_master): Device \"%s\": not compatible with profile \"%s\", set profile=\"default\"",
 el->eng->dev1_name(), cprof);

 }
 el->is_updated = 1;
 return 0;
 }
 } while (0);

 PDEBUG(DFULL,"Initialize device");
 EOC_dev_terminal *dev = init_dev(name);
 if (!dev){
 PDEBUG(DERR,"dev = %p",dev);
 return -1;
 }

 PDEBUG(DFULL,"Initialize config");
 EOC_config *cfg = new EOC_config(&conf_profs, cprof, app_cfg);
 PDEBUG(DERR, "CONFIG=%p,cprof=%s", cfg, cfg->cprof());
 channel_elem *el = new channel_elem(dev, cfg);
 el->name = name;
 el->nsize = strlen(name);
 el->is_updated = 1;
 PDEBUG(DFULL,"%s pointer = %p",el->name,el);
 channels.add(el);
 channels.sort();
 return 0;
 }

 int EOC_main::add_master(char *name, char *cprof, char *aprof, int reps,
 int tick_per_min, int pbomode, char *pboval, int app_cfg) {
 PDEBUG(DERR, "start");

 {
 PDEBUG(DFULL,"Try to opendir");
 DIR *dir;
 if ( dir = opendir(OS_IF_PATH)) {
 PDEBUG(DFULL,"opendir - success; close");
 closedir(dir);
 }
 }


 do {
 PDEBUG(DERR, "call channels.find");
 channel_elem *el = (channel_elem*) channels.find(name, strlen(name));
 PDEBUG(DERR, "el=%p", el);
 if (el) { // If this device exist in current configuration
 // If type of interface is changed
 if (el->eng->get_type() != master) {
 PDEBUG(DERR, "Reinit device %s", name);
 PDEBUG(DFULL,"%s pointer = %p",el->name,el);
 channel_elem *d = (channel_elem*) channels.del(name, strlen(
 name));
 delete d;
 break;
 }
 // Check if some of optins changed
 EOC_config *cfg = ((EOC_engine_act*) el->eng)->config();
 // Repeaters number
 cfg->repeaters(reps);
 // Configuration profile name
 cfg->cprof(cprof);
 if (el->eng->check_compat()) {
 char *p = strdup("default");
 cfg->cprof(p);
 syslog(
 LOG_ERR,
 "eocd(add_master): Device \"%s\": not compatible with profile \"%s\", set profile=\"default\"",
 el->eng->dev1_name(), cprof);
 PDEBUG(
 DERR,
 "eocd(add_master): Device \"%s\": not compatible with profile \"%s\", set profile=\"default\"",
 el->eng->dev1_name(), cprof);

 }
 // Alarm profile name
 cfg->aprof(aprof);
 // eocd apply capability
 cfg->can_apply(app_cfg);
 el->is_updated = 1;
 return 0;
 }
 } while (0);

 {
 PDEBUG(DFULL,"Try to opendir");
 DIR *dir;
 if ( dir = opendir(OS_IF_PATH)) {
 PDEBUG(DFULL,"opendir - success; close");
 closedir(dir);
 }
 }

 PDEBUG(DERR, "call init_dev");
 EOC_dev_terminal *dev = (EOC_dev_terminal *) init_dev(name);
 PDEBUG(DERR, "dev=%p", dev);
 if (!dev) {
 syslog(LOG_ERR, "eocd(add_master): Cannot initialize device \"%s\"",
 name);
 PDEBUG(DERR, "eocd(add_master): Cannot initialize device \"%s\"",
 name);
 return -1;
 }

 EOC_config *cfg = new EOC_config(&conf_profs, &alarm_profs, cprof, aprof,
 reps, app_cfg);

 // Check compatibility
 conf_profile *prof = (conf_profile*) cfg->conf();
 if (dev->comp() < prof->comp) {
 syslog(
 LOG_ERR,
 "eocd(add_master): Device \"%s\": not compatible with profile \"%s\", set profile=\"default\"",
 dev->if_name(), cprof);
 PDEBUG(
 DERR,
 "eocd(add_master): Device \"%s\": not compatible with profile \"%s\", set profile=\"default\"",
 dev->if_name(), cprof);
 char *p = strdup("default");
 cfg->cprof(p);
 }

 PDEBUG(DERR, "CONFIG=%p", cfg);

 channel_elem *el = new channel_elem(dev, cfg, tick_per_min);
 el->name = name;
 el->nsize = strlen(name);
 el->is_updated = 1;
 PDEBUG(DFULL,"%s pointer = %p",el->name,el);
 ((EOC_engine_act*) el->eng)->set_pbo(pbomode, pboval);
 channels.add(el);
 channels.sort();
 PDEBUG(DERR, "FINISH");
 return 0;
 }

 // NOTE: this function is only used in non-KDB mode
 // at this time KDB mode is preferred so code can be out of date
 int EOC_main::add_nexist_slave(int pcislot, int pcidev, char *cprof,
 int app_cfg) {
 list<dev_settings_t>::iterator it = nexist_devs.begin();
 for (; it != nexist_devs.end(); it++) {
 if (it->pcislot == pcislot && it->pcidev == pcidev) {
 // Devise already exist in list - IGNORE
 return -1;
 }
 }
 dev_settings_t tmp;
 tmp.pcislot = pcislot;
 tmp.pcidev = pcidev;
 tmp.cprof = cprof;
 tmp.master = 0;
 tmp.can_apply = app_cfg;
 nexist_devs.push_back(tmp);
 return 0;
 }

 // NOTE: this function is only used in non-KDB mode
 // at this time KDB mode is preferred so code can be out of date
 int EOC_main::add_nexist_master(int pcislot, int pcidev, char *cprof,
 char *aprof, int repeaters, int pbomode, char *pboval, int eocd_apply) {
 list<dev_settings_t>::iterator it = nexist_devs.begin();
 for (; it != nexist_devs.end(); it++) {
 if (it->pcislot == pcislot && it->pcidev == pcidev) {
 // Devise already exist in list - IGNORE
 return -1;
 }
 }
 dev_settings_t tmp;
 tmp.pcislot = pcislot;
 tmp.pcidev = pcidev;
 tmp.master = 1;
 tmp.cprof = cprof;
 tmp.aprof = aprof;
 tmp.reps = repeaters;
 tmp.can_apply = eocd_apply;
 tmp.pbomode = pbomode;
 strncpy(tmp.pboval, pboval, PBO_SETTING_LEN);
 nexist_devs.push_back(tmp);
 return 0;

 }

 int EOC_main::configure_channels() {
 channel_elem *el = (channel_elem*) channels.first();
 int ret = 0;
 err_cfgchans_cnt = 0;
 while (el) {
 int tmp = 0;
 PDEBUG(DERR, "Configure %s channel", el->name);

 // Configure interface
 if ((tmp = el->eng->configure(el->name)) && err_cfgchans_cnt
 < SPAN_NAMES_NUM) {
 err_cfgchans[err_cfgchans_cnt++] = el;
 }
 ret += tmp;
 el = (channel_elem*) channels.next(el->name, el->nsize);
 }
 return ret;
 }

 int EOC_main::poll_channels() {
 channel_elem *el = (channel_elem*) channels.first();
 while (el) {
 el->eng->schedule(el->name);
 el = (channel_elem*) channels.next(el->name, el->nsize);
 }
 }

 // ------------ Application requests ------------------------------//

 void EOC_main::app_listen() {
 char *buff;
 int size, conn;
 int ret;
 time_t start, cur;
 time(&start);
 cur = start;
 int seconds = 60 / tick_per_min; // Count number of seconds need to wait

 while (time(&cur) > 0 && time(&cur) - start < seconds) {
 int to_wait = seconds - (time(&cur) - start);
 if (!app_srv.wait(to_wait)) {
 continue;
 }
 PDEBUG(DERR, "Get new message:");
 while ((size = app_srv.recv(conn, buff))) {
 PDEBUG(DERR, "Recv new message:");
 // Form & check incoming request
 app_frame fr(buff, size);
 PDEBUG(DERR, "Process new message:");
 // Fill response payload
 if (!fr.is_negative()) {
 if (ret = app_request(&fr)) {
 fr.negative();
 PDEBUG(DERR, "Failed to serv app request");
 }
 }
 // Form response
 fr.response();
 // Send response
 ret = app_srv.send(conn, fr.frame_ptr(), fr.frame_size());
 }
 time(&cur);
 }
 }

 int EOC_main::app_request(app_frame *fr) {
 if (fr->role() != app_frame::REQUEST)
 return -1;

 PDEBUG(DERR, "App request, ID = %d", fr->id());
 switch (fr->id()) {
 case APP_SPAN_NAME:
 return app_spanname(fr);
 case APP_SPAN_CONF:
 PDEBUG(DERR, "Span Conf case");
 return app_spanconf(fr);
 case APP_CPROF:
 PDEBUG(DINFO, "APP_SPAN_CPROF");
 return app_cprof(fr);
 case APP_LIST_CPROF:
 PDEBUG(DINFO, "APP_SPAN_CPROF_LIST");
 return app_list_cprof(fr);
 case APP_PBO:
 PDEBUG(DINFO, "APP_PBO");
 return app_chan_pbo(fr);

 #ifndef KDB_CONFIG
 // Now this code is old and need revision to work properly
 case APP_ADD_CPROF:
 PDEBUG(DINFO, "APP_ADD_CPROF");
 return app_add_cprof(fr);
 case APP_DEL_CPROF:
 PDEBUG(DINFO, "APP_DEL_CPROF");
 return app_del_cprof(fr);
 case APP_ADD_CHAN:
 PDEBUG(DINFO, "APP_ADD_CHAN");
 return app_add_chan(fr);
 case APP_DEL_CHAN:
 PDEBUG(DINFO, "APP_DEL_CHAN");
 return app_del_chan(fr);
 case APP_CHNG_CHAN:
 PDEBUG(DINFO, "APP_CHANG_CHAN");
 return app_chng_chan(fr);
 case APP_DUMP_CFG: {
 int ret;
 PDEBUG(DERR, "APP_DUMP_CFG");
 ret = write_config();
 if (ret) {
 fr->negative(-ret);
 }
 return 0;
 }
 #endif // KDB_CONFIG
 }

 PDEBUG(DINFO, "Channel request");
 if (fr->chan_name())
 return app_chann_request(fr);
 return -1;
 }

 int EOC_main::app_spanname(app_frame *fr) {
 span_name_payload *p = (span_name_payload*) fr->payload_ptr();

 switch (fr->type()) {
 case APP_GET:
 case APP_GET_NEXT: {
 channel_elem *el = (channel_elem*) channels.first();
 if (!el) {
 fr->negative(ERCHNEXIST);
 return 0;
 }
 if (strnlen(fr->chan_name(), SPAN_NAME_LEN)) {
 // if first name isn't zero
 el = (channel_elem *) channels.find((char*) fr->chan_name(),
 strnlen(fr->chan_name(), SPAN_NAME_LEN));
 if (!el) {
 fr->negative(ERCHNEXIST);
 return 0;
 }
 el = (channel_elem*) channels.next(el->name, el->nsize);
 }
 int filled = 0;
 while (el && filled < SPAN_NAMES_NUM) {
 int cp_len = (el->nsize > SPAN_NAME_LEN) ? SPAN_NAME_LEN
 : el->nsize;
 strncpy(p->spans[filled].name, el->name, cp_len);
 p->spans[filled].t = el->eng->get_type();
 p->spans[filled].comp = el->eng->get_compat();
 filled++;
 el = (channel_elem*) channels.next(el->name, el->nsize);
 }
 p->filled = filled;
 p->last_msg = (el && (filled = SPAN_NAMES_NUM)) ? 0 : 1;
 return 0;
 }
 }
 fr->negative(ERTYPE);
 return 0;
 }

 int EOC_main::app_spanconf(app_frame *fr) {
 span_conf_payload *p = (span_conf_payload*) fr->payload_ptr();

 channel_elem *el;

 if (strnlen(fr->chan_name(), SPAN_NAME_LEN)) {
 // if first name isn't zero
 el = (channel_elem *) channels.find((char*) fr->chan_name(), strnlen(
 fr->chan_name(), SPAN_NAME_LEN));
 if (!el) {
 fr->negative(ERCHNEXIST);
 return 0;
 }
 } else {
 fr->negative(ERCHNEXIST);
 }

 EOC_config *cfg = el->eng->config();
 PDEBUG(DERR, "cfg=%p", el->eng->config());

 switch (fr->type()) {
 case APP_GET:
 case APP_GET_NEXT: {
 PDEBUG(DERR, "SPAN_CONF:");
 p->type = el->eng->get_type();
 p->nreps = cfg->repeaters();
 PDEBUG(DERR, "rep = %d", cfg->repeaters());
 if (cfg->cprof())
 PDEBUG(DERR, "conf prof = (%p)%s", cfg->cprof(), cfg->cprof());
 if (cfg->aprof())
 PDEBUG(DERR, "alarm prof = %s", cfg->aprof());
 strncpy(p->conf_prof, cfg->cprof(), SNMP_ADMIN_LEN);
 if (!cfg->aprof()) {
 strncpy(p->alarm_prof, "no_profile", SNMP_ADMIN_LEN);
 } else {
 strncpy(p->alarm_prof, cfg->aprof(), SNMP_ADMIN_LEN);
 }
 PDEBUG(DERR, "SPAN_CONF:");
 break;
 }
 }
 return 0;
 }

 int EOC_main::app_chann_request(app_frame *fr) {
 // check that requested channel exist
 channel_elem *el = (channel_elem *) channels.find((char*) fr->chan_name(),
 strnlen(fr->chan_name(), SPAN_NAME_LEN));
 if (!el) { // No such channel on this device
 fr->negative(ERCHNEXIST);
 return 0;
 }
 EOC_engine *eng = el->eng;

 if (eng->get_type() != master) { // Channel do not maintain EOC DB
 fr->negative(ERNODB);
 return 0;
 }
 EOC_engine_act *eng_a = (EOC_engine_act *) eng;
 PDEBUG(DERR, "Engine request");
 eng_a->app_request(fr);
 return 0;
 }

 // -------------- Span configuration profiles requests ----------------//

 int EOC_main::app_cprof(app_frame *fr) {
 cprof_payload *p = (cprof_payload*) fr->payload_ptr();
 span_conf_profile_t *mconf = &p->conf;
 int len = strnlen(p->pname, SNMP_ADMIN_LEN + 1);

 PDEBUG(DERR, "Start");

 conf_profile * prof;
 switch (fr->type()) {
 case APP_GET:
 if (!len) {
 fr->negative(ERPARAM);
 return 0;
 }
 prof = (conf_profile *) conf_profs.find(p->pname, len);
 if (!prof) { // No such profile
 fr->negative(ERPNEXIST);
 return 0;
 }
 p->conf = prof->conf;
 p->comp = prof->comp;
 return 0;
 case APP_GET_NEXT:
 PDEBUG(DERR, "GET_NEXT");
 if (!len) { // requested first entry
 PDEBUG(DERR, "First entry");
 prof = (conf_profile *) conf_profs.first();
 } else {
 PDEBUG(DERR, "Next after %s", p->pname);
 prof = (conf_profile *) conf_profs.next(p->pname, len);
 }
 if (!prof) {
 PDEBUG(DERR, "prof = 0");
 fr->negative(ERPNEXIST);
 return 0;
 }
 PDEBUG(DERR, "Gen response");
 p->conf = prof->conf;
 p->comp = prof->comp;
 strncpy(p->pname, prof->name, prof->nsize + 1);
 return 0;
 #ifndef KDB_CONFIG
 // Now this code is old and need revision to work properly
 case APP_SET: {
 // Requested profile exist?
 prof = (conf_profile*) conf_profs.find(p->pname, len);
 if (!prof) { // Profile not exist - cannot change
 fr->negative(ERPNEXIST);
 PDEBUG(DERR, "Requested profile \"%s\" not exist", p->pname);
 return 0;
 } else if (prof->access == conf_profile::profRO) {
 PDEBUG(DERR, "Profile %s is read only!", p->pname);
 fr->negative(ERPRONLY);
 return 0;
 }
 span_conf_profile_t *pconf = &prof->conf;
 span_conf_profile_t pconf_bkp = *pconf;
 // Changes
 cprof_changes *c = (cprof_changes *) fr->changelist_ptr();

 if (c->annex) {
 pconf->annex = mconf->annex;
 }
 if (c->wires) {
 pconf->wires = mconf->wires;
 }
 if (c->power) {
 pconf->power = mconf->power;
 }
 if (c->psd) {
 pconf->psd = mconf->psd;
 }
 if (c->clk) {
 pconf->clk = mconf->clk;
 }
 if (c->line_probe) {
 pconf->line_probe = mconf->line_probe;
 }
 if (c->remote_cfg) {
 pconf->remote_cfg = mconf->remote_cfg;
 }
 if (c->use_cur_down) {
 pconf->use_cur_down = mconf->use_cur_down;
 }
 if (c->use_worst_down) {
 pconf->use_worst_down = mconf->use_worst_down;
 }
 if (c->use_cur_up) {
 pconf->use_cur_up = mconf->use_cur_up;
 }
 if (c->use_worst_up) {
 pconf->use_worst_up = mconf->use_worst_up;
 }

 if (c->rate) {
 // Round rate value to be 64Kbps ratio
 pconf->rate = (int) (mconf->rate / 64) * 64;
 }
 if (c->tcpam) {
 pconf->tcpam = mconf->tcpam;
 }
 if (c->cur_marg_down) {
 pconf->cur_marg_down = mconf->cur_marg_down;
 }
 if (c->worst_marg_down) {
 pconf->worst_marg_down = mconf->worst_marg_down;
 }
 if (c->cur_marg_up) {
 pconf->cur_marg_up = mconf->cur_marg_up;
 }
 if (c->worst_marg_up) {
 pconf->worst_marg_up = mconf->worst_marg_up;
 }
 PDEBUG(DERR, "Commit Changes");
 if (configure_channels()) {
 PDEBUG(DERR, "Cannot commit changes, chans_cnt=%d",
 err_cfgchans_cnt);
 for (int i = 0; i < err_cfgchans_cnt; i++) {
 strncpy(p->names[i], err_cfgchans[i]->name, SPAN_NAME_LEN);
 }
 p->filled = err_cfgchans_cnt;
 fr->negative(ERPNCOMP);
 *pconf = pconf_bkp;
 configure_channels();
 }
 return 0;
 }
 #endif
 }
 }

 int EOC_main::app_list_cprof(app_frame *fr) {
 cprof_list_payload *p = (cprof_list_payload*) fr->payload_ptr();

 switch (fr->type()) {
 case APP_GET:
 case APP_GET_NEXT: {
 conf_profile * prof;
 int len;
 PDEBUG(DERR, "start,pname[0][0]=%d", (int) p->pname[0][0]);
 if ((len = strnlen(p->pname[0], SNMP_ADMIN_LEN))) {
 // if first name isn't zero
 PDEBUG(DERR, "requested prof=%s\n", p->pname[0]);
 prof = (conf_profile *) conf_profs.find(p->pname[0], len);
 if (!prof) {
 fr->negative(ERPNEXIST);
 return 0;
 }
 PDEBUG(DERR, "prof-name=%s", prof->name);
 prof = (conf_profile*) conf_profs.next(prof->name, prof->nsize);
 } else {
 PDEBUG(DERR, "prof-name=NULL");
 prof = (conf_profile *) conf_profs.first();
 if (!prof) {
 fr->negative(ERPNEXIST);
 return 0;
 }
 }
 if (prof)
 PDEBUG(DERR, "Fill response for %s", prof->name);
 int filled = 0;
 while (prof && filled < PROF_NAMES_NUM) {
 int cp_len = (prof->nsize > SNMP_ADMIN_LEN) ? SNMP_ADMIN_LEN
 : prof->nsize + 1;
 strncpy(p->pname[filled], prof->name, cp_len + 1);
 PDEBUG(DERR, "add %s,cp_len=%d", prof->name, cp_len);
 filled++;
 prof = (conf_profile*) conf_profs.next(prof->name, prof->nsize);
 }
 p->filled = filled;
 p->last_msg = (prof && (filled == PROF_NAMES_NUM)) ? 0 : 1;
 for (int i = 0; i < filled; i++)
 PDEBUG(DERR, "pname[%d]=%s", i, p->pname[i]);
 return 0;
 }
 }
 fr->negative(ERTYPE);
 return 0;
 }

 int EOC_main::app_chan_pbo(app_frame *fr) {
 // check that requested channel exist
 channel_elem *el = (channel_elem *) channels.find((char*) fr->chan_name(),
 strnlen(fr->chan_name(), SPAN_NAME_LEN));
 char buf[3 * 16];
 int mode = -1;
 char *val = "";

 PDEBUG(DERR, "start");
 if (!el || el->eng->get_type() == slave) {
 PDEBUG(DERR, "Channel %s,el=%p", fr->chan_name(), el);
 fr->negative(ERCHNEXIST);
 return 0;
 }

 PDEBUG(DERR, "go through");

 EOC_engine_act *eng = (EOC_engine_act *) el->eng;
 chan_pbo_payload *p = (chan_pbo_payload *) fr->payload_ptr();

 PDEBUG(DERR, "switch");

 #ifndef KDB_CONFIG
 // Now this code is old and need revision to work properly

 switch (fr->type()) {
 case APP_SET: {
 PDEBUG(DERR, "APP_SET");
 chan_pbo_changes *c = (chan_pbo_changes *) fr->changelist_ptr();
 if (c->mode)
 mode = p->mode;
 if (c->val)
 val = p->val;
 eng->set_pbo(mode, val);
 configure_channels();
 }
 }
 #endif // KDB_CONFIG
 PDEBUG(DERR, "APP_GET");
 eng->get_pbo(mode, p->val);
 PDEBUG(DERR, "#2");
 p->mode = mode;

 return 0;
 }

 #ifndef KDB_CONFIG
 // Now this code is old and need revision to work properly

 int EOC_main::app_add_cprof(app_frame *fr) {
 cprof_add_payload *p = (cprof_add_payload*) fr->payload_ptr();

 switch (fr->type()) {
 case APP_SET:
 break;
 default: // Only set operation
 fr->negative(ERTYPE);
 return 0;
 }

 // check that adding profile not exist already
 int len = strnlen(p->pname, SNMP_ADMIN_LEN + 1);
 if (!len) {
 fr->negative(ERPARAM);
 return 0;
 }
 conf_profile *prof = (conf_profile*) conf_profs.find(p->pname, len);
 if (prof) { // Profile already exist - cannot create
 fr->negative(ERPEXIST);
 return 0;
 }

 // Get default profile
 prof = (conf_profile*) conf_profs.find("default", 7);
 conf_profile *nprof = new conf_profile;
 memset(nprof, 0, sizeof(conf_profile));
 if (!(nprof->name = strndup(p->pname, len))) {
 // Not enough memory
 fr->negative(ERNOMEM);
 return 0;
 }
 nprof->nsize = len;
 // Default configuration
 nprof->conf = prof->conf;

 // Save compatibility settings
 if (p->cap > (u8) EOC_dev::cap_max)
 p->cap = (u8) EOC_dev::cap_uni;
 nprof->cap = (EOC_dev::capability_t) p->cap;

 // Add profile to data base
 conf_profs.add(nprof);
 conf_profs.sort();
 return 0;
 }

 int EOC_main::app_del_cprof(app_frame *fr) {
 cprof_del_payload *p = (cprof_del_payload*) fr->payload_ptr();
 switch (fr->type()) {
 case APP_SET:
 break;
 default: // Only set operation
 fr->negative(ERTYPE);
 return 0;
 }

 // check that adding profile not exist already
 int len = strnlen(p->pname, SNMP_ADMIN_LEN + 1);
 if (!len) {
 PDEBUG(DERR, "Profile name zero len");
 fr->negative(ERPNEXIST);
 return 0;
 }

 PDEBUG(DERR, "Find deleting profile");
 conf_profile *prof = (conf_profile*) conf_profs.find(p->pname, len);
 PDEBUG(DERR, "Find success, prof=%p", prof);
 if (prof == NULL) { // Profile not exist so cannot delete
 PDEBUG(DERR, "Profile %s not exist", p->pname);
 fr->negative(ERPNEXIST);
 return 0;
 } else if (prof->access == conf_profile::profRO) {
 PDEBUG(DERR, "Profile %s is read only!", p->pname);
 fr->negative(ERPRONLY);
 return 0;
 }
 PDEBUG(DERR, "Find channels");
 // Check that no channel is associated with this profile
 channel_elem *el = (channel_elem *) channels.first();
 while (el) {
 if (el->eng->get_type() == master) {
 const char *cprof = ((EOC_engine_act*) el->eng)->config()->cprof();
 if (!strncmp(prof->name, cprof, SNMP_ADMIN_LEN)) {
 // Cannot delete profile - associated with this channel
 PDEBUG(DERR, "Profile %s is busy", prof->name);
 fr->negative(ERPBUSY);
 return 0;
 }
 }
 el = (channel_elem *) channels.next(el->name, el->nsize);
 }
 el = (channel_elem *) conf_profs.del(prof->name, prof->nsize);
 delete el;
 return 0;
 }

 int EOC_main::app_add_chan(app_frame *fr) {
 int len = strnlen(fr->chan_name(), SPAN_NAME_LEN);
 char *name = strndup(fr->chan_name(), len);
 chan_add_payload *p = (chan_add_payload *) fr->payload_ptr();

 switch (fr->type()) {
 case APP_SET:
 break;
 default: // Only set operation
 fr->negative(ERTYPE);
 return 0;
 }

 // check that requested channel exist
 channel_elem *el = (channel_elem *) channels.find((char*) fr->chan_name(),
 strnlen(fr->chan_name(), SPAN_NAME_LEN));
 if (el) { // Channel already exist
 fr->negative(ERCHEXIST);
 return 0;
 }

 // Assign default profile to new channel
 hash_elem *pel = conf_profs.find("default", 7);
 if (!pel) {
 // No configuration profiles!
 fr->negative(ERPNEXIST);
 return 0;
 }
 char *cprof = strndup(pel->name, SNMP_ADMIN_LEN + 1);
 if (!cprof) {
 syslog(LOG_ERR, "Not enought memory");
 PDEBUG(DERR, "Not enought memory");
 fr->negative(ERNOMEM);
 return 0;
 }

 if (!p->master) {
 if (add_slave(name, cprof, 1)) {
 fr->negative(ERNODEV);
 return 0;
 }
 } else {
 if (add_master(name, cprof, NULL, 0, tick_per_min, 0, "",
 1)) {
 syslog(LOG_ERR, "(%s): cannot add channel \"%s\" - no such device",
 config_file, name);
 PDEBUG(DERR, "(%s): cannot add channel \"%s\" - no such device",
 config_file, name);
 fr->negative(ERNODEV);
 return 0;
 }
 }

 configure_channels();
 return 0;
 }

 int EOC_main::app_del_chan(app_frame *fr) {
 // check that requested channel exist
 channel_elem *el = (channel_elem *) channels.find((char*) fr->chan_name(),
 strnlen(fr->chan_name(), SPAN_NAME_LEN));

 switch (fr->type()) {
 case APP_SET:
 break;
 default: // Only set operation
 fr->negative(ERTYPE);
 return 0;
 }

 if (!el) { // Nothing to delete
 fr->negative(ERCHNEXIST);
 return 0;
 }
 el = channels.del(el->name, el->nsize);
 delete el;
 configure_channels();
 return 0;
 }

 int EOC_main::app_chng_chan(app_frame *fr) {
 u8 new_changes = 0;
 int ret = 0;
 switch (fr->type()) {
 case APP_SET:
 break;
 default: // Only set operation
 fr->negative(ERTYPE);
 return 0;
 }

 PDEBUG(DERR, "Start");

 // check that requested channel exist
 channel_elem *el;
 el = (channel_elem *) channels.find((char*) fr->chan_name(), strnlen(
 fr->chan_name(), SPAN_NAME_LEN));
 if (!el) { // Nothing to change
 PDEBUG(DERR, "No such channel %s", fr->chan_name());
 fr->negative(ERCHNEXIST);
 return 0;
 }
 chan_chng_payload *p;
 p = (chan_chng_payload *) fr->payload_ptr();

 char *name = strndup(el->name, el->nsize);

 char *cprof = strndup(el->eng->config()->cprof(), SNMP_ADMIN_LEN + 1);

 hash_elem *prof =
 conf_profs.find(cprof, strnlen(cprof, SNMP_ADMIN_LEN + 1));
 if (!prof) {
 // No configuration profiles!
 PDEBUG(DERR, "No configuration profiles");
 fr->negative(ERNOPROF);
 return 0;
 }

 // Change channel status
 if (p->master_ch) {
 if (el->eng->get_type() == master && !p->master) {
 char *name = strndup(el->name, el->nsize);
 PDEBUG(DERR, "Add slave: %s, %s", name, cprof);
 if (add_slave(name, cprof, 1)) {
 fr->negative(ERNODEV);
 return 0;
 }
 configure_channels();
 return 0;
 } else if (el->eng->get_type() == slave && p->master) {
 if (add_master(name, cprof, NULL, 0, tick_per_min, 0, "", 1)) {
 syslog(LOG_ERR,
 "(%s): cannot add channel \"%s\" - no such device",
 config_file, name);
 PDEBUG(DERR,
 "(%s): cannot add channel \"%s\" - no such device",
 config_file, name);
 fr->negative(ERNODEV);
 return 0;
 }
 new_changes++;
 }
 el = (channel_elem *) channels.find((char*) fr->chan_name(), strnlen(
 fr->chan_name(), SPAN_NAME_LEN));
 if (!el) { // Nothing to change
 PDEBUG(DERR, "No such channel %s", fr->chan_name());
 fr->negative(ERCHNEXIST);
 return 0;
 }
 }

 PDEBUG(DERR, "Change configuration");
 EOC_config *cfg = ((EOC_engine_act*) el->eng)->config();
 PDEBUG(DERR, "Set repeaters num");
 if (p->rep_num_ch) {
 new_changes++;
 if (cfg->repeaters(p->rep_num))
 ret = -1;
 }

 PDEBUG(DERR, "Set conf profle");
 if (p->cprof_ch) {
 new_changes++;
 PDEBUG(DERR, "Cprof=%s", p->cprof);
 if (!conf_profs.find(p->cprof, strnlen(p->cprof, SNMP_ADMIN_LEN))) {
 syslog(LOG_ERR, "Wrong configuration profile name %s", p->cprof);
 PDEBUG(DERR, "No such profile=%s", p->cprof);
 ret = -1;
 } else {
 char *cprof = strndup(p->cprof, SNMP_ADMIN_LEN);
 cfg->cprof(cprof);
 }
 }

 PDEBUG(DERR, "Set can apply");
 if (p->apply_conf_ch) {
 new_changes++;
 cfg->can_apply(p->apply_conf);
 }

 if (new_changes) {
 PDEBUG(DERR, "Configure channels");
 if (configure_channels()) {
 cfg->cprof_revert();
 configure_channels();
 fr->negative(ERPNCOMP);
 }
 } else if (ret) {
 fr->negative(ERPARAM);
 }
 return 0;
 }*/
