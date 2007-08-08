#include <stdio.h>
#include <unistd.h>
#include <db/EOC_loop.h>

int main()
{
    EOC_loop loop;
    side_perf p;
    memset(&p,0,sizeof(p));
    p.es = 1;
    p.ses = 10;
    int k = 0;
    while(1){
	printf("new iter:\n");
//	sleep(10);
	loop.full_status(&p);
	k++;
    p.es += 1;
    p.ses += 10;

	if( !(k%6) )
	    loop.print_15m();
	if( k == 30 )
	    break;
    }
    
}