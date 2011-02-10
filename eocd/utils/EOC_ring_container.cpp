/*
 * stat_ring.c
 * Provide interface for storing statistical data wich
 * has fixed length. New arriving data pull old data out.
 */

#include <stdio.h>
#include <utils/EOC_ring_container.h>

int main()
{
    EOC_ring_container<int> ints(32);

    int k=0;
    *ints[0] = k;
    for(k=1;k<32;k++){
	ints.shift(1);
	*ints[0] = k;
    }
    for(k=0;k<32;k++){
	if( ints[k] )
	    printf("int #%d es=%d\n",k,*ints[k]);
    }

    k=50;
    *ints[0] = k;
    for(k=51;k<82;k++){
	ints.shift(1);
	*ints[0] = k;
    }
    for(k=0;k<32;k++){
	if( ints[k] )
	    printf("int #%d es=%d\n",k,*ints[k]);
    }


}


