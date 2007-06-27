#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>

#include <devs/EOC_sg17.h>

EOC_sg17::EOC_sg17(char *file)
{
/*
    char *opts[]={"annex","crate","master","mod","rate","remcfg",
		    "state","statistic"};
    int opts_num = sizeof(opts)/sizeof(char*);		    
    cpath = strdup(p);
    if( check_ctrl_files() ){
	error_init = 1;
	return;
    }
*/
    int fd;

    if( (fd = open(file,O_RDWR)) < 0 ){
        valid = 0;
        return;
    }
    fname = strndup(file,256);
    valid = 1;
}

EOC_sg17::~EOC_sg17()
{
    free(fname);
}


#define BUFF_SZ 1024
int
EOC_sg17::send(EOC_msg *m)
{
    int fd;
    int wrcnt = 0;
    int err = 0;
    
    if( !valid )
	return -1;

    if( (fd = open(fname,O_WRONLY) ) < 0 ){
	valid = 0;
	return -1;
    }
    
    wrcnt = write(fd,m->mptr(),m->msize());
    if( wrcnt < m->msize() )
	err = -1;

    close(fd);
    return err;
}
    
    
EOC_msg *
EOC_sg17::recv()
{
    int fd;
    char buff[BUFF_SZ];
    char *ptr;
    int rdcnt=0,cnt = 0;
    int rdbytes=0;
    EOC_msg *msg;

    if( !valid )
	return NULL;

    if( (fd = open(fname,O_RDONLY) ) < 0 ){
	valid = 0;
	return NULL;
    }
    
    rdbytes = ((BUFF_SZ-rdcnt) > HDLC_BUFF_SZ) ? HDLC_BUFF_SZ : (BUFF_SZ-rdcnt);
    while ( rdbytes && (cnt=read(fd,(buff+rdcnt),rdbytes)) ){
	rdcnt += cnt;
	rdbytes = (BUFF_SZ-rdcnt > HDLC_BUFF_SZ) ? HDLC_BUFF_SZ : BUFF_SZ-rdcnt;
    }
    close(fd);

    if( !rdcnt )
	return NULL;
    
    ptr = (char*)malloc(sizeof(char) * rdcnt);
    memcpy(ptr,buff,rdcnt);
    msg = new EOC_msg;
    if( msg->setup(ptr,rdcnt) ){
	free(ptr);
	return NULL;
    }

/* FOR DEBUG PURPOSE */
    FILE *stream = fopen(fname,"w");
    if( stream )
	fclose(stream);
/* DEBUG END */    
    return msg;
}
