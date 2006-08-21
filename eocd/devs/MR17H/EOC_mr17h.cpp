#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <dirent.h>
#include <syslog.h>

#include <generic/EOC_generic.h>
#include <devs/EOC_mr17h.h>
#include <eoc_debug.h>

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
	PDEBUG(DERR,"check_ctrl failed\n");
	syslog(LOG_ERR,"Failed to find MR17H control files in %s",conf_path);
	return;
    }
    chan_path = new char[FILE_PATH_SIZE];
    snprintf(chan_path,FILE_PATH_SIZE,"%s/eoc",conf_path);
    PDEBUG(DINFO,"MR17H device (%s) successflly initialized\n",name);
    valid = 1;
    
    PDEBUG(DINFO,"finish");
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
	PDEBUG(DERR,"Error - initialisation unsuccessfull\n");
	return -1;
    }

    if( (fd = open(chan_path,O_WRONLY) ) < 0 ){	
	PDEBUG(DERR,"Cannot open file: %d\n",chan_path);
	valid = 0;
	return -1;
    }
    
    wrcnt = write(fd,m->mptr(),m->msize());
    if( wrcnt < m->msize() ){
	PDEBUG(DERR,"Sended %d must be %d\n",wrcnt,m->msize());
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
	PDEBUG(DERR,"Devise not valid");
	return NULL;
    }

    if( (fd = open(chan_path,O_RDONLY) ) < 0 ){
	PDEBUG(DERR,"Cannot open file %s",chan_path);    
	valid = 0;
	return NULL;
    }
    cnt=read(fd,buff,BUFF_SZ);
    close(fd);
    PDEBUG(DFULL,"Raded %d bytes",cnt);
    if( !cnt )
	return NULL;
    
    ptr = (char*)malloc(cnt);
    memcpy(ptr,buff,cnt);
    msg = new EOC_msg;
    if( msg->setup(ptr,cnt) ){
	PDEBUG(DERR,"Error wile setting up msg");
	free(ptr);
	return NULL;
    }
    return msg;
}

int EOC_mr17h::
set_dev_option(char *name,char *val)
{
    int fd;
    char fname[FILE_PATH_SIZE];
    if( !valid )
	return -1;
	
    PDEBUG(DFULL,"set_dev_option(%s,%s)",name,val); 
    snprintf(fname,FILE_PATH_SIZE,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_WRONLY)) < 0 ){
	PDEBUG(DERR,"set_dev_option: Cannot open %s",fname);
	return -1;
    }
    int len = strlen(val)+1;
    int cnt = write(fd,val,len);
    PDEBUG(DFULL,"set_dev_option: write %d, written %d",len,cnt);
    close(fd);
    if( cnt != len )
	return -1;
    return 0;
}

int EOC_mr17h::
get_dev_option(char *name,char *&buf)
{
    const int BUF_SIZE=256;
    int fd;
    char fname[FILE_PATH_SIZE];

    if( !valid )
	return -1;

    buf = new char[BUF_SIZE];
    PDEBUG(DFULL,"get_dev_option(%s)",name); 
    snprintf(fname,FILE_PATH_SIZE,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_RDONLY)) < 0 ){
	PDEBUG(DERR,"get_dev_option: Cannot open %s",fname);
	delete[] buf;
	buf = NULL;
	return -1;
    }
    int cnt = read(fd,buf,BUF_SIZE);
    PDEBUG(DFULL,"get_dev_option: readed %d",cnt);
    close(fd);
    if( cnt < 0 ){
	delete[] buf;
	buf = NULL;
	return 0;
    }else if( !cnt ){
	delete[] buf;
	buf = NULL;
	return 0;
    }
    buf[cnt] = 0;
    return cnt;
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
    if( set_dev_option("mode","1") )
	return -1;
    switch( cfg.annex ){
    case annex_a:
	if( set_dev_option("annex","0") )
	    return -1;
	break;
    case annex_b:
	if( set_dev_option("annex","1") )
	    return -1;
	break;
    default:
	PDEBUG(DERR,"Unexpected annex value: %d, set to annex_a",cfg.annex);
	if( set_dev_option("annex","0") )
	    return -1;
    }	
    // TODO: pay attension to MIN rate!!!!
    snprintf(setting,256,"%d",cfg.max_rate);
    if( set_dev_option("rate",setting) )
	return -1;
    // Уточнить значение!!!!
    if( cfg.max_rate > 3840 ){
	if( set_dev_option("tcpam","1") )
	    return -1;
    }else {
	if( set_dev_option("tcpam","0") )
	    return -1;
    }

    // Apply configuration    
    if( set_dev_option("apply_cfg","1") )
	return -1;
        
}

// Configure device as STU-R
int EOC_mr17h::
configure()
{
    // Setup
    if( set_dev_option("mode","0") )
	return -1;
    // Apply configuration    
    if( set_dev_option("apply_cfg","1") )
	return -1; 
}

// DEBUG_VERSION
EOC_dev::Linkstate
EOC_mr17h::link_state()
{
    char *buf;
    int state = 0;
    int cnt = get_dev_option("link_state",buf);
    if( cnt <= 0 )
	return OFFLINE;
    int params = sscanf(buf,"%d",&state);
    delete[] buf;
    if( params != 1 )
	return OFFLINE;
    if( state ){
	PDEBUG(DFULL,"GET LINK UP\n");
	return ONLINE;
    }
    return OFFLINE;
}

int EOC_mr17h::
statistics(int loop,side_perf &stat)
{
    char *buf;
    u8 ovfl,rst;
    int ret = 0;
    int cnt = get_dev_option("statistics_row",buf);
    if( cnt <= 0 )
	return -1;
    PDEBUG(DFULL,"STATISTICS: read params");
    int params = sscanf(buf,"%d %d %u %u %u %u %u %*u %*u %u %u",
	    &stat.snr_marg,&stat.loop_attn,&stat.es,&stat.ses,&stat.crc,
	    &stat.losws,&stat.uas,&rst,&ovfl);
    PDEBUG(DFULL,"STATISTICS: readed %d params",params);
    delete[] buf;
    if( params != 9 )
	return -1;
    stat.cntr_rst_stur = stat.cntr_rst_stuc = rst;
    stat.cntr_ovfl_stur = stat.cntr_ovfl_stuc = ovfl;

    PDEBUG(DFULL,"snr(%d) attn(%d) es(%d) ses(%d) crc(%d) losws(%d) uas(%d) cntr_rst(%d) ovfl(%d)",
	    stat.snr_marg,stat.loop_attn,stat.es,stat.ses,stat.crc,
	    stat.losws,stat.uas,stat.cntr_rst_stuc,stat.cntr_ovfl_stuc);
    
    side_perf tmp = stat;
    tmp.snr_marg = last_perf.snr_marg;
    if( memcmp(&tmp,&last_perf,sizeof(last_perf)) ){
	ret = 1;
    }
    if( stat.snr_marg < snr_tresh && snr_tresh ){
	stat.snr_marg_alarm = 1;
	ret = 1;
    }
    if( stat.loop_attn < attn_tresh && attn_tresh ){
	stat.loop_attn_alarm = 1;
	ret = 1;
    }

    last_perf = stat;
    return ret;
}

