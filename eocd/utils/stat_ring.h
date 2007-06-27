#ifndef SIGRAND_STAT_RING_H
#define SIGRAND_STAT_RING_H

typedef struct {
    void **mas;
    int len;
    int head,qlen;
} Eocd_ring;

// functions 
Eocd_ring *eocd_ring_alloc(int len);
void eocd_ring_free(Eocd_ring *r);
void *eocd_ring_get(Eocd_ring *r,int ind);
void eocd_ring_insert(Eocd_ring *r,void *elem);

#endif

