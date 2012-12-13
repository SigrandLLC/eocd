#include <devs/EOC_mr17h.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>

int main()
{
    EOC_mr17h dev("dsl0");
    EOC_msg *m;
    if( !(m = dev.recv()) ){
	printf("ERROR: while recv\n");
	return 0;
    }
    printf("src(%d),dst(%d),type(%d)\nContain: ",m->src(),m->dst(),m->type());
    for(int i=0;i<11;i++){
	printf("%c ",m->mptr()[i]);
    }
    printf("\nEND\n");
    return 0;
}