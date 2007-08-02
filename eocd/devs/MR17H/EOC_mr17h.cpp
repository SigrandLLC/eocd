#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <dirent.h>

#include <generic/EOC_generic.h>
#include <devs/EOC_mr17h.h>
#include <eocd_log.h>

/* NOW member of EOC_dev_terminal
int
check_ctrl_files(char *d,char **opts,int opts_num) 
{
    DIR *dir;
    int ocnt = 0;
    struct dirent *ent;
    
    if( !(dir = opendir(d)) ){
	printf("Cannot open dir: %s\n",d);
	return -1;
    }
    
    for(int i=0;i<opts_num;i++){
	rewinddir(dir);
	while( ent = readdir(dir) ){
//	    printf("check equal of: %s - %s\n",opts[i],ent->d_name);
	    if( !strncmp(opts[i],ent->d_name,256) ){
		printf("%s - %s is equal\n",opts[i],ent->d_name);
		ocnt++;
		break;
	    }
	}
    }
    closedir(dir);
    printf("ocnt = %d, opts_num = %d\n",ocnt,opts_num);
    return (ocnt == opts_num ) ? 0 : -1;	
}

*/
EOC_mr17h::
EOC_mr17h(char *name)
{
    char *opts[]={"annex","mode","rate","statistics","eoc"};
    int opts_num = sizeof(opts)/sizeof(char*);		    
    DIR *dir;

    ifname = strndup(name,256);
    conf_path = new char[PATH_SIZE];    
    chan_path = NULL;
    valid = 0;
    mr17h_conf_dir(name,conf_path,PATH_SIZE);

    if( check_ctrl_files(conf_path,opts,opts_num) ){
	printf("check_ctrl failed\n");
	return;
    }
    chan_path = new char[FILE_PATH_SIZE];
    snprintf(chan_path,FILE_PATH_SIZE,"%s/eoc",conf_path);
    printf("Init mr17h - success\n");
    valid = 1;
}

EOC_mr17h::
~EOC_mr17h()
{
    delete[] conf_path;
    if(chan_path)
	delete[] chan_path;
}


#define BUFF_SZ 1024
int EOC_mr17h::
send(EOC_msg *m)
{
    int fd;
    int wrcnt = 0;
    int err = 0;
    
    if( !valid ){
	printf("Error - initialisation unsuccessfull\n");
	return -1;
    }

    if( (fd = open(chan_path,O_WRONLY) ) < 0 ){	
	printf("Cannot open file: %d\n",chan_path);
	valid = 0;
	return -1;
    }
    
    wrcnt = write(fd,m->mptr(),m->msize());
    if( wrcnt < m->msize() ){
	printf("Sended %d must be %d\n",wrcnt,m->msize());
	err = -1;
    }

    close(fd);
    return err;
}
    
    
EOC_msg *EOC_mr17h::
recv()
{
    int fd;
    char buff[BUFF_SZ];
    char *ptr;
    int rdcnt=0,cnt = 0;
    int rdbytes=0;
    EOC_msg *msg;

    if( !valid ){
	printf("Devise not valid\n");
	return NULL;
    }

    if( (fd = open(chan_path,O_RDONLY) ) < 0 ){
	printf("Cannot open file %s\n",chan_path);    
	valid = 0;
	return NULL;
    }
    cnt=read(fd,buff,BUFF_SZ);
    close(fd);
//    printf("Raded %d bytes\n",cnt);
    if( !cnt )
	return NULL;
    
    ptr = (char*)malloc(cnt);
    memcpy(ptr,buff,cnt);
    msg = new EOC_msg;
    if( msg->setup(ptr,cnt) ){
	printf("Error wile setting up msg\n");
	free(ptr);
	return NULL;
    }
    return msg;
}

int EOC_mr17h::
set_config_elem(char *name,char *val)
{
    int fd;
    char fname[FILE_PATH_SIZE];
    printf("set_config_elem(%s,%s)\n",name,val); 
    snprintf(fname,FILE_PATH_SIZE,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_WRONLY)) < 0 ){
	printf("set_config_elem: Cannot open %s\n",fname);
	return -1;
    }
    int len = strlen(val)+1;
    int cnt = write(fd,val,len);
    printf("set_config_elem: write %d, written %d\n",len,cnt);
    close(fd);
    if( cnt != len )
	return -1;
    return 0;
}

span_conf_profile_t *EOC_mr17h::
cur_config()
{
    return NULL;

}

// Configure device as STU-C
int EOC_mr17h::
configure(span_conf_profile_t &cfg)
{
    char setting[256];

    // Setup
    if( set_config_elem("mode","1") )
	return -1;
    switch( cfg.annex ){
    case annex_a:
	if( set_config_elem("annex","0") )
	    return -1;
	break;
    case annex_b:
	if( set_config_elem("annex","1") )
	    return -1;
	break;
    default:
	eocd_log(CONFL,"Unexpected annex value: %d, set to annex_a\n",cfg.annex);
	if( set_config_elem("annex","0") )
	    return -1;
    }	
    // TODO: pay attension to MIN rate!!!!
    snprintf(setting,256,"%d",cfg.max_rate);
    if( set_config_elem("rate",setting) )
	return -1;
    // Уточнить значение!!!!
    if( cfg.max_rate > 3840 ){
	if( set_config_elem("tcpam","1") )
	    return -1;
    }else {
	if( set_config_elem("tcpam","0") )
	    return -1;
    }

    // Apply configuration    
    if( set_config_elem("apply_cfg","1") )
	return -1;
        
}

// Configure device as STU-R
int EOC_mr17h::
configure()
{
    // Setup
    if( set_config_elem("mode","0") )
	return -1;
    // Apply configuration    
    if( set_config_elem("apply_cfg","1") )
	return -1; 
}

// DEBUG_VERSION
EOC_dev::Linkstate
EOC_mr17h::link_state(){ return ONLINE; }
// END DEBUG_VERSION
/*
/*
    int status_collect(){
	1;2A    return 0;
    }

    int perf_change(u8 loop){
	if(perf_changed){
	    perf_changed = 0;
	    return 1;
	}
	return 0;
    }
	    
    
    u8 losws_alarm(u8 loop){
	return perf.losws_alarm;
    }
    
    u8 loop_attn_alarm(u8 loop){
	return perf.loop_attn_alarm;
    }
    
    u8 snr_marg_alarm(u8 loop){
	return perf.snr_marg_alarm;
    }
    u8 dc_cont_flt(u8 loop){
	return perf.dc_cont_flt;
    }
    u8 dev_flt(u8 loop){
	return perf.dev_flt;
    }
    u8 pwr_bckoff_st(u8 loop){
	return perf.pwr_bckoff_st;
    }
    
    s8 snr_marg(u8 loop){
	return perf.snr_marg;
    }
    s8 loop_attn(u8 loop){
	return perf.loop_attn;
    }
    
    u8 es(u8 loop){
	return perf.es;
    }
    u8 ses(u8 loop){
	return perf.ses;
    }
    u8 crc(u8 loop){
	return perf.crc;
    }
    u8 losws(u8 loop){
	return perf.losws;
    }
    u8 uas(u8 loop){
	return perf.uas;
    }
    u8 pwr_bckoff_base_val(u8 loop){
	return perf.pwr_bckoff_base_val;
    }
    u8 cntr_rst_scur(u8 loop){
	return perf.cntr_rst_scur;
    }
    u8 cntr_ovfl_stur(u8 loop){
	return perf.cntr_ovfl_stur;
    }
    u8 cntr_rst_scuc(u8 loop){
	return perf.cntr_rst_scuc;
    }
    u8 cntr_ovfl_stuc(u8 loop){
	return perf.cntr_ovfl_stuc;
    }
    u8 pwr_bkf_ext(u8 loop){
	return perf.pwr_bkf_ext;
    }
    
    shdsl_config config(){ shdsl_config i; return i; }
    int config(shdsl_config cfg){ 
	printf("SET DEVICE: Rate=%d %s annex%d",cfg.lrate, (cfg.master) ? "master" : "slave",cfg.annex);
	return 0;
    }
*/

int EOC_mr17h::
statistics(side_perf &stat)
{


}