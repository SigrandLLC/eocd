#include <sys/types.h>
#include <dirent.h>
#include <devs/EOC_dev_terminal.h>

int EOC_dev_terminal::
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
