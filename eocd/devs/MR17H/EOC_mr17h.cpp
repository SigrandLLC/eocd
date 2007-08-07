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
set_dev_option(char *name,char *val)
{
    int fd;
    char fname[FILE_PATH_SIZE];
    if( !valid )
	return -1;
	
    printf("set_dev_option(%s,%s)\n",name,val); 
    snprintf(fname,FILE_PATH_SIZE,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_WRONLY)) < 0 ){
	printf("set_dev_option: Cannot open %s\n",fname);
	return -1;
    }
    int len = strlen(val)+1;
    int cnt = write(fd,val,len);
    printf("set_dev_option: write %d, written %d\n",len,cnt);
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
    printf("get_dev_option(%s)\n",name); 
    snprintf(fname,FILE_PATH_SIZE,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_WRONLY)) < 0 ){
	printf("get_dev_option: Cannot open %s\n",fname);
	delete[] buf;
	buf = NULL;
	return -1;
    }
    int cnt = read(fd,buf,BUF_SIZE);
    printf("get_dev_option: readed %d\n",cnt);
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
	eocd_log(CONFL,"Unexpected annex value: %d, set to annex_a\n",cfg.annex);
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
EOC_mr17h::link_state(){ return ONLINE; }

int EOC_mr17h::
statistics(int loop,side_perf &stat)
{
    char *buf;
    int cnt = get_dev_option("statistics_row",buf);
    if( cnt <= 0 )
	return -1;
    int params = sscanf(buf,"%d %d %u %u %u %u %u %*u %*u %u %u",
	    stat.snr_marg,stat.loop_attn,stat.es,stat.ses,stat.crc,
	    stat.losws,stat.uas,stat.cntr_rst_stuc,stat.cntr_ovfl_stuc);
    if( params != 11 )
	return -1;
    stat.cntr_rst_stur = stat.cntr_rst_stuc;
    stat.cntr_ovfl_stur = stat.cntr_ovfl_stuc;

    return 0;
}

