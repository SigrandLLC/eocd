#ifndef SCHED_QUEUE_H
#define SCHED_QUEUE_H

#include <generic/EOC_generic.h>
#include <list>
using namespace std;

class __timestamp{
protected:
    unsigned long ticks;
    enum {IMMEDIATELY=0};
public:
    __timestamp(){ ticks = 0; }
    __timestamp(unsigned int t){ ticks = t; }
    __timestamp(const __timestamp &t){ ticks = t.ticks; }
    __timestamp(__timestamp t,unsigned int offs){ ticks = t.ticks + offs; }

    inline bool operator ==(__timestamp &right){
	if( ticks == right.ticks )
	    return 1;
	return 0;
    }
    inline bool operator < (__timestamp &right){
	if( ticks < right.ticks )
	    return 1;
	return 0;
    }
    inline bool operator <= (__timestamp &right){
	if( ticks <= right.ticks )
	    return 1;
	return 0;
    }
    inline __timestamp & operator ++ (int k){
	ticks++;
	return *this;
    }

};

class sched_elem{
public:
    unit src,dst;
    unsigned char type;
    __timestamp tstamp;
public:
    inline bool operator < (sched_elem &right){
	if( tstamp < right.tstamp )
	    return 1;
	return 0;
    }
        
};


class sched_queue{
public:
protected:
    list<sched_elem> q;
public:
    sched_queue(){
	q.clear();
    }
    ~sched_queue(){
	q.clear();
    }

    inline int add(unit src,unit dst,unsigned char type,__timestamp ts){
	list<sched_elem>::iterator p = q.begin();
	list<sched_elem>::iterator p1 = q.end();
	for(;p != q.end();p++ ){
	    if( src == p->src && dst == p->dst && type == p->type )
		return -1;
	    if( p->tstamp == ts )
		p1 = p;
	}
	sched_elem *n = new sched_elem;
	n->src = src;
	n->dst = dst;
	n->type = type;
	n->tstamp = ts;
	q.insert(p1,*n);
	delete n;
    }
    
    inline int schedule(unit &src,unit &dst, unsigned char &type,__timestamp cur){
	q.sort();
	list<sched_elem>::iterator p = q.begin();
	for(;p != q.end();p++ ){
	    if( p->tstamp <= cur ){
		src = p->src;
		dst = p->dst;
		type = p->type;
		q.erase(p);
		return 0;
	    }
	}	
	return -1;
    }
    inline int find_del(unit src,unit dst,unsigned char type){
	q.sort();
	list<sched_elem>::iterator p = q.begin();
	for(;p != q.end();p++ ){
	    unit s1 = p->src;
	    unit d1 = p->dst;
	    char t1 = p->type;
	    if( (src == p->src) && (dst == p->dst) && (type == p->type) ){
		q.erase(p);
		return 0;
	    }
	}	
	return -1;
    }
    inline void clear(){
	q.clear();
    }
};

/*
int main()
{
    __timestamp t(200);
    __timestamp t1(t);
    __timestamp t2(t,5);
    __timestamp t3(195);
    sched_queue q;
    unit s,d;
    unsigned char tp;
    q.add(stu_c,stu_r,1,t);
    q.add(stu_c,sru1,1,t2);
    q.add(stu_c,sru2,2,t2);
    t++;
    int k=q.find_del(stu_c,sru3,1);
    k=q.find_del(stu_c,sru2,1);
    k=q.find_del(stu_c,sru2,2);
    k=q.find_del(stu_c,sru1,1);
    k=q.find_del(stu_c,stu_r,1);
    k=q.find_del(stu_c,stu_r,1);

}
*/
#endif
