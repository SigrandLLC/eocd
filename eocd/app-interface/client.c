#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>

#include <unix_interface.h>

int
main()
{
    char *b,ch,a;
    int i;
    Eocd_desc *d = eocd_alloc_desc();
    Eocd_msg *msg = eocd_alloc_msg(10);
    
    if( !msg ){
	printf("No memory!\n");
	return 0;
    }
    eocd_setup_msg(msg,1,2,0,8);
    eocd_set_msg_status(msg,REQUEST);
    if( eocd_init_client(d) ){
	return (0);
    }
    for(i=0;i<2;i++){
	b = eocd_get_dataptr(msg);
	for(ch = 'a'+i*10; ch<='a'+i*10+10;ch++)
	    b[ch-'a'-i*10] = ch;
	eocd_send_msg(d->fd[0],msg);
	printf("second part?:");
	scanf("%c",&a);
    }
    close(d->fd[0]);
    return 0;
}
