#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>

#include "include/unix_interface.h"

main()
{
    Eocd_desc *desc = eocd_alloc_desc();
    Eocd_msg *msg;
    int cnt=0,i;
    int fd;
    char *buff;
    
    eocd_init_server(desc);
    while(1){
	    if( !(cnt = eocd_wait(desc)) )
		continue;
	    for(i=0;i<cnt;i++){
		fd = eocd_nex_fd(desc);
		eocd_recv_msg(fd,&msg);
		if( !msg )
			continue;
		printf("REquest %d:%d to change device %d:%d:\nMessage content:\n",
			msg->id,msg->sub_id,msg->span,msg->dev);
		buff = eocd_get_dataptr(msg);			
		for(i=0;i<msg->len;i++)
			printf("%c",buff[i]);
		printf("\n---------------END----------------\n");
	    }
    }

    exit(0);
}
