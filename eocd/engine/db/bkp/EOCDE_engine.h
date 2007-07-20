#ifndef SIGRAND_EOCDB_ENGINE_H
#define SIGRAND_EOCDB_ENGINE_H

#include <sys/types.h>
#include "../utils/EOCLog.h"
#define MAX_PATH 200

class EOCDB_engine{
    EOCList *spans;
    EOCLog *log;
    int reconf_req;
public:
    inline EOCDB_engine(){
	spans = new EOCList;
    }
    inline ~EOCDB_engine(){
	delete spans;
    }
    int main();
    int spans_init();
    int sg16_spans();
    int sg17_spans();    
    int span_poll(EOCSpan *s);
};


EOCDB_engine::EOCDB_engine(EOCLog_conf *log_cfg,EOCSpan_conf *span_cfg)
{
    log = new EOCLog(LOG_MAIN,log_cfg);
    spans = new EOCList(span_cfg);
    reconf_req = 0;
}


EOCDB_engine::~EOCDB_engine(){
	delete spans;
	delete log;
}


int
EOCDB_engine::main()
{
    EOCSpan *s;
    int err=0;    

    // Main Loop
    while(1){
	// 1. Read configuration from file
    
	// 2. Search all aviliable channels
        if( spans_init() )
	    break;
    
	// service channels
	while( !reconf_req ){
	    if( !spans->head_current() ){
		log->print0(ERROR,"Span list setup error");
		return -1;
	    }
	    do{
		s = (EOCSpan *)spans->get_cur());
		if( err = span_poll(s){
		    log->print1(ERROR,"Span %s polling error",s->name());
		    return -1;
		}
	    }while( !spans->next() );
	}
    }
}


int
EOCDB_engine::spans_init()
{
    // 1. Find SG16 spans
    

}

int
EOCDB_engine::sg16_spans()
{
    char dname[] = "/sys/bus/pci/drivers/sg16lan/";
    DIR *ddesc;
    struct dirent *dent;
    
    EOCSG16_dev *d;
    
    ddesc = opendir(p);
    if( !ddesc ){
//	log->print(ERROR,"%s",strerror(errno));
	printf("%s\n",strerror(errno));
	return -1;
    }
    while( (dent = readdir(ddesc) ) != NULL ){
	if( !strstr(dent->d_name,"dsl") )
	    continue;
	d = new EOCSG16_dev(dent->d_name);
	printf("%s\n",dent->d_name);
    }    
    closedir(ddesc);
    return 0;    
}

int
EOCDB_engine::sg17_spans()
{
    char dname[] = "/sys/class/net/";
    char dname1[MAX_PATH];
    DIR *ddesc1,*ddesc2;
    struct dirent *dent1,*dent2;
    EOCSG17_dev *d;
    
    ddesc1 = opendir(p);
    if( !ddesc1 ){
//	log->print(ERROR,"%s",strerror(errno));
	printf("%s\n",strerror(errno));
	return -1;
    }
    while( (dent1 = readdir(ddesc1) ) != NULL ){
	if( !strstr(dent1->d_name,"dsl") )
	    continue;
	snprintf(dname2,"%s%s",dname,dent1->d_name);
	if( (ddesc2 = opendir(ddesc2)) == NULL ){
	    printf("Warn: %s\n",strerror(errno));
	    continue;
	}
	while( (dent2 = readdir(ddesc2)) != NULL ){
	    if( !strstr(dent1->d_name,"sg_priv") )
		continue;
	    d = new EOCSG17_dev(dent1->d_name);	    
	    printf("%s\n",dent1->d_name);	
	}
	closedir(ddesc2);
    }    
    closedir(ddesc1);
    return 0;    
}




#endif
