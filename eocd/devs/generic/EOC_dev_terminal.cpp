#include <sys/types.h>
#include <dirent.h>
#include <devs/EOC_dev_terminal.h>
#include <eoc_debug.h>

int EOC_dev_terminal::
check_ctrl_files(char *d,char **opts,int opts_num) 
{
    DIR *dir;
    int ocnt = 0;
    struct dirent *ent;
    
    if( !(dir = opendir(d)) ){
	PDEBUG(DERR,"Cannot open dir: %s",d);
	return -1;
    }
    
    for(int i=0;i<opts_num;i++){
	rewinddir(dir);
	while( ent = readdir(dir) ){
	    PDEBUG(DFULL,"check equal of: %s - %s",opts[i],ent->d_name);
	    if( !strncmp(opts[i],ent->d_name,256) ){
		PDEBUG(DFULL,"%s - %s is equal",opts[i],ent->d_name);
		ocnt++;
		break;
	    }
	}
    }
    closedir(dir);
    PDEBUG(DFULL,"ocnt = %d, opts_num = %d",ocnt,opts_num);
    return (ocnt == opts_num ) ? 0 : -1;	
}
