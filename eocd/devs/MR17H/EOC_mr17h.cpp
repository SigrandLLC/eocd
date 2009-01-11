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
#define EOC_DEBUG
#include <eoc_debug.h>


EOC_mr17h::
EOC_mr17h(char *name)
{
    char *opts[]={"annex","mode","rate","statistics","eoc","chipver"};
    int opts_num = sizeof(opts)/sizeof(char*);
    DIR *dir;
	char *ver;

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

	valid = 1;

	if( get_dev_option("chipver",ver) <= 0 ){
		valid = 0;
		return;
	}
	if( !strncmp("v1",ver,sizeof("v1")) ){
		_comp = comp_base;
	}else if( !strncmp("v2",ver,sizeof("v2")) ){
		_comp = comp_ext1;
	}
	delete[] ver;

    PDEBUG(DINFO,"MR17H device (%s) ver %d successflly initialized\n",name,_comp);
    PDEBUG(DINFO,"finish");
}

EOC_mr17h::
~EOC_mr17h()
{
    delete[] conf_path;
    if(chan_path)
		delete[] chan_path;
}

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
    char buff[MSG_BUFSZ];
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
    cnt=read(fd,buff,MSG_BUFSZ);
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
    int fd;
    char fname[FILE_PATH_SIZE];

    if( !valid )
		return -1;

    buf = new char[OPT_BUFSZ];
    PDEBUG(DFULL,"get_dev_option(%s)",name);
    snprintf(fname,FILE_PATH_SIZE,"%s/%s",conf_path,name);
    if( (fd = open(fname,O_RDONLY)) < 0 ){
		PDEBUG(DERR,"get_dev_option: Cannot open %s",fname);
		delete[] buf;
		buf = NULL;
		return -1;
    }
    int cnt = read(fd,buf,MSG_BUFSZ);
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


int EOC_mr17h::
cur_config(span_conf_profile_t &cfg,int &mode)
{
	char *buf;
	int cnt;

	// Device mode
    if( (cnt = get_dev_option("mode",buf)) < 0 )
		return -1;

	if( !strncmp(buf,"master",cnt) )
		mode = 1;
	else if( !strncmp(buf,"slave",cnt) )
		mode = 0;
	else{
		// incorrect mode setup (just in case)
		mode = -1;
		delete[] buf;
		return -1;
	}
	delete[] buf;
	PDEBUG(DERR,"Mode=%d",mode);

	// Device ANNEX setup
	if( (cnt = get_dev_option("annex",buf)) < 0 )
		return -1;
	if( !strncmp(buf,"A",cnt) )
		cfg.annex = annex_a;
	else if( !strncmp(buf,"B",cnt) )
		cfg.annex = annex_b;
	else{
		PDEBUG(DERR,"Unknown ANNEX value %s",buf);
		delete[] buf;
		return -1;
	}
	delete[] buf;
	PDEBUG(DERR,"annex=%d",cfg.annex);

	// Device rate value
	char *endp;
	if( (cnt=get_dev_option("rate",buf)) < 0 )
		return -1;
	if( cnt ){
		cfg.rate = strtoul(buf,&endp,0);
		if( buf == endp )
			return -1;
		delete[] buf;

	}else{
		cfg.rate = 0;
	}
	PDEBUG(DERR,"Rate=%d",cfg.rate);

	// Device ANNEX setup
	if( (cnt=get_dev_option("tcpam",buf)) < 0 )
		return -1;

	if( !strncmp(buf,"TCPAM4",cnt) )
		cfg.tcpam = tcpam4;
	else if( !strncmp(buf,"TCPAM8",cnt) )
		cfg.tcpam = tcpam8;
	else if( !strncmp(buf,"TCPAM16",cnt) )
		cfg.tcpam = tcpam16;
	else if( !strncmp(buf,"TCPAM32",cnt) )
		cfg.tcpam = tcpam32;
	else if( !strncmp(buf,"TCPAM64",cnt) )
		cfg.tcpam = tcpam64;
	else if( !strncmp(buf,"TCPAM128",cnt) )
		cfg.tcpam = tcpam128;
	else{
		PDEBUG(DERR,"Unknown TCPAM value %s",buf);
		delete[] buf;
		return -1;
	}
	delete[] buf;
	PDEBUG(DERR,"tcpam=%d",cfg.tcpam);

	cfg.power = noPower;
	// Device ANNEX setup
	if( (cnt=get_dev_option("pwron",buf)) < 0 )
		return -1;
	if( !strncmp(buf,"0",cnt) )
		cfg.power = noPower;
	else if( !strncmp(buf,"1",cnt) )
		cfg.power = powerFeed;
	else{
		PDEBUG(DERR,"Unknown power value %s",buf);
		delete[] buf;
		return -1;
	}
	delete[] buf;
	PDEBUG(DERR,"power=%d",cfg.power);
	return 0;
}

// Configure device as STU-C
int EOC_mr17h::
configure(span_conf_profile_t &cfg, int type,int &need_commit)
{
    char setting[256];
	span_conf_profile_t ocfg;
	int mode;
	need_commit = 0;
	int ret = 0;

	PDEBUG(DERR,"Master configuration of %s",ifname);
	if( !cur_config(ocfg,mode) ){
		if( type == 1 ){ // For master
			if( mode == 1 && cfg.annex == ocfg.annex && cfg.rate == ocfg.rate
					&& cfg.power == ocfg.power && cfg.tcpam == ocfg.tcpam ){
				PDEBUG(DERR,"Device configuration left unchanged");
				return 0;
			}
		}else{ // for slave
			if( mode == 0 && cfg.power == ocfg.power ){
				PDEBUG(DERR,"Device configuration left unchanged");
				return 0;
			}
		}
	}else{
		PDEBUG(DERR,"Cannot read current config from dev");
	}

	PDEBUG(DERR,"Configure device");

    // Setup
	if( mode != type ){
		if( type ){
			if( set_dev_option("mode","1") )
				return -1;
		}else{
			if( set_dev_option("mode","0") )
				return -1;
		}
		need_commit++;
	}

	switch( cfg.power ){
	case noPower:
		set_dev_option("pwron","0");
		break;
	case powerFeed:
		set_dev_option("pwron","1");
		break;
	}

	if( !type )
		goto complete;

	need_commit++;

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
    snprintf(setting,256,"%d",cfg.rate);
    if( set_dev_option("rate",setting) ){
		syslog(LOG_ERR,"Error while setting rate to %d",(int)cfg.tcpam);
		return -1;
	}

	// TODO: Change debug level to DINFO */
	PDEBUG(DERR,"Set TCPAM to %d",(int)cfg.tcpam);
	snprintf(setting,256,"%d",(int)cfg.tcpam);
	if( set_dev_option("tcpam",setting) ){
		PDEBUG(DERR,"Error while setting TCPAM to %d",(int)cfg.tcpam);
		syslog(LOG_ERR,"Error while setting TCPAM to %d",(int)cfg.tcpam);
	}
	if( !cur_config(ocfg,mode) ){
		if( cfg.tcpam != ocfg.tcpam || cfg.rate != ocfg.rate ){
			ret = 1;
		}
	}

 complete:
    // Apply configuration
	return ret;
}

int EOC_mr17h::
commit()
{
	PDEBUG(DERR,"COMMIT");
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


// Power Backoff
int EOC_mr17h::
get_pbo(int &mode,char *buf)
{
    char *tmp;
	char str[256];

	// Mode
	PDEBUG(DERR,"MODE");
    int cnt = get_dev_option("pbo_mode",tmp);
    if( cnt <= 0 )
		return -1;
	if( !strcmp(tmp,"Forced") ){
		mode = 1;
	}else if( !strcmp(tmp,"Normal") ){
		mode = 0;
	}else{
		PDEBUG(DERR,"Unexpected value of pbo_val: %s",tmp);
		syslog(LOG_ERR,"Unexpected value of pbo_val: %s",tmp);
	    delete[] tmp;
		return -1;
	}
    delete[] tmp;

	// Value
	PDEBUG(DERR,"VALUE");
    cnt = get_dev_option("pbo_val",tmp);
    if( cnt <= 0 )
		return -1;
	strncpy(buf,tmp,PBO_SETTING_LEN);
    delete[] tmp;

	PDEBUG(DERR,"end");

    return 0;
}

int EOC_mr17h::
set_pbo(int &mode,char *buf)
{
    char tmp[256];

	// Mode
	PDEBUG(DERR,"MODE");
	if( mode >= 0 ){
		sprintf(tmp,"%d",mode);
    	if( set_dev_option("pbo_mode",tmp) ){
			PDEBUG(DERR,"Error while setting pbo_mode to %d",mode);
			syslog(LOG_ERR,"Error while setting pbo_mode to %d",mode);
		}
	}

	// Value
	PDEBUG(DERR,"Value: %p",buf);
	if( buf ){
	    if( set_dev_option("pbo_val",buf) ){
			PDEBUG(DERR,"Error while setting pbo_val to %d",mode);
			syslog(LOG_ERR,"Error while setting pbo_val to %d",mode);
		}
	}
    return 0;
}
