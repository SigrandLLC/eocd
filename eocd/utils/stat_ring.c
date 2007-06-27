/*
 * stat_ring.c
 * Provide interface for storing statistical data wich 
 * has fixed length. New arriving data pull old data out.
 */
 
#include <stdio.h>
#include "../include/utils/stat_ring.h"

static int
ring_sub(int from,int what,int len)
{
    return ( (from>=what) ? (from-what) : (len-(what-from)) );
}

static int
ring_add(int from,int what,int len)
{
    return ( ((from+what) < len) ? (from+what) : ((what+from)-len) );
}

Eocd_ring *
eocd_ring_alloc(int len)
{
    Eocd_ring *r = (Eocd_ring*)malloc(sizeof(Eocd_ring));
    r->len = len;
    r->head = r->len-1;
    r->qlen = 0;
    r->mas = malloc(sizeof(void*)*(r->len));
    memset(r->mas,0,sizeof(void*)*(r->len));
    return r;
}

void
eocd_ring_free(Eocd_ring *r)
{
    free(r->mas);
    free(r);
}

void *
eocd_ring_get(Eocd_ring *r,int ind)
{
    if( ind > r->qlen )
	return NULL;
    ind = ring_sub(r->head,ind,r->len);
    return r->mas[ind];
}

void
eocd_ring_insert(Eocd_ring *r,void *elem)
{
    r->head = ring_add(r->head,1,r->len);    
    r->mas[r->head] = elem;
    if(r->qlen < (r->len))
	r->qlen++;
}

/*
int main()
{
    int *a;
    int i,j,b[1000];
    Eocd_ring *r = eocd_ring_alloc(30);
    
    for(i=0;i<1000;i++)
	    b[i] = i;
    for(i=0;i<30;i++)
	eocd_ring_insert(r,(void*)(b+i));
    for(i=30;i<100;i++){
	    printf("Iter %d: ",i-30);
	    for(j=0;j<30;j++){
		a = (int *)eocd_ring_get(r,j);
		printf("%d ",*a);
	    }
	    printf("\n");
	    eocd_ring_insert(r,(void*)(b+i));
    }
    return 0;
}
*/
