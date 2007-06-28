#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>

#include <devs/EOC_dummy.h>

EOC_dummy::EOC_dummy(char *file1,char *file2)
{
    int fd;

    if( (fd = open(file1,O_RDWR)) < 0 ){
        valid = 0;
        return;
    }
    close(fd);
    if( (fd = open(file2,O_RDWR)) < 0 ){
        valid = 0;
        return;
    }
    close(fd);

    f1 = strndup(file1,256);
    f2 = strndup(file2,256);    
    valid = 1;
}

EOC_dummy::~EOC_dummy()
{
    free(f1);
    free(f2);    
}


#define BUFF_SZ 1024
int
EOC_dummy::send(EOC_msg *m)
{
    int fd;
    int wrcnt = 0;
    int err = 0;
    
    if( !valid )
	return -1;

    if( (fd = open(f1,O_WRONLY) ) < 0 ){
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
EOC_dummy::recv()
{
    int fd;
    char buff[BUFF_SZ];
    char *ptr;
    int rdcnt=0,cnt = 0;
    int rdbytes=0;
    EOC_msg *msg;

    if( !valid )
	return NULL;

    if( (fd = open(f2,O_RDONLY) ) < 0 ){
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
    FILE *stream = fopen(f2,"w");
    if( stream )
	fclose(stream);
/* DEBUG END */    
    return msg;
}

// DEBUG_VERSION
EOC_dev::Linkstate
EOC_dummy::link_state(){ return ONLINE; }
// END DEBUG_VERSION
