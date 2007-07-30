#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include<errno.h>
#include <string.h>

#include <app_comm_cli.h>
#include <app_frame.h>

int
main()
{
    char *b,ch,a;
    int i;
    app_comm_cli cli("./sock");
    app_frame fr(app_frame::SPAN_STATUS,app_frame::GET,app_frame::REQUEST,"dsl0");
    
    for(i=0;i<10;i++){
	cli.send(fr.frame_ptr(),fr.frame_size());
	printf("second part?:");
//	scanf("%c",&a);
    }
    return 0;
}
