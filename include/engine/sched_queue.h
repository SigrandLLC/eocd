#ifndef SCHED_QUEUE_H
#define SCHED_QUEUE_H

#include <generic/EOC_generic.h>
#include <engine/sched_elem.h>
#include <list>

using namespace std;

class sched_queue{
 public:
 protected:
    list<sched_elem> q;
    u32 err_no_answer;
 public:
    sched_queue(){
		err_no_answer = 0;
		q.clear();
    }
    ~sched_queue(){
		q.clear();
    }

    void clear(){
		q.clear();
    }

    int add(unit src,unit dst,unsigned char type,__timestamp ts){
		list<sched_elem>::iterator p = q.begin();
		list<sched_elem>::iterator p1 = q.end();
		for(;p != q.end();p++ ){
			if( src == p->src && dst == p->dst && type == p->type )
				return -1;
			if( p->tstamp == ts )
				p1 = p;
		}
		if( p1 != q.end() )
			p1++;
		sched_elem *n = new sched_elem;
		n->src = src;
		n->dst = dst;
		n->type = type;
		n->tstamp = ts;
		q.insert(p1,*n);
		delete n;
		return 0;
    }

    inline int add(sched_elem &el){
		return add(el.src,el.dst,el.type,el.tstamp);
    }

    int get_old(__timestamp cur,u32 wait_to,sched_elem &el)
    {
		if( !q.size() )
			return -1;

		q.sort();
		list<sched_elem>::iterator p = q.begin();

		if( cur - p->tstamp > wait_to ){
			el = *p;
			//	    printf("FIND_DEL: clear src(%d) dst(%d) type(%d) tstamp(%d)\n",p->src,p->dst,p->type,p->tstamp.get_val());
			q.pop_front();
			return 0;
		}
		return -1;
    }

    int schedule(sched_elem &el,__timestamp cur){
		q.sort();
		unit swap;
		list<sched_elem>::iterator p = q.begin();
		for(;p != q.end();p++ ){
			if( p->tstamp <= cur ){
				el = *p;
				q.erase(p);
				return 0;
			}else
				return -1;
		}
		return -1;
    }

    int find_del(unit src,unit dst,unsigned char type,__timestamp cur){
		q.sort();
		list<sched_elem>::iterator p = q.begin();
		for(;p != q.end();p++ ){
			// search needed element
			unit s1 = p->src;
			unit d1 = p->dst;
			char t1 = p->type;
			if( ((src == p->src)||(p->src == BCAST)) && (dst == p->dst) && (type == p->type) ){
				q.erase(p);
				return 0;
			}
		}
		return -1;
    }

    void print(){
		q.sort();
		list<sched_elem>::iterator p = q.begin();
		int i=0;
		for(;p != q.end();p++ ){
			unsigned char a = p->type;
			printf("%d: src(%d),dst(%d),type(%u),tick(%d)\n",i,p->src,p->dst,(unsigned char)(a&0xff),p->tstamp.get_val());
			i++;
		}
    }

};

/*
  int main()
  {
  __timestamp t(200);
  __timestamp t1(t);
  __timestamp t2(t,5);
  __timestamp t3(195);
  sched_queue q,q1;

  unit s,d;
  unsigned char tp;
  sched_elem el;
  int k=q.schedule(el,t2,q1);
  q.add(stu_c,stu_r,1,t);
  q.add(stu_c,sru1,1,t2+10);
  q.add(stu_c,sru2,2,t2);
  t++;
  k=q.find_del(stu_c,sru3,1);
  k=q.schedule(el,t2,q1);
  k=q1.find_del(stu_c,sru2,2);
  k=q.find_del(stu_c,sru1,1);
  k=q.find_del(stu_c,stu_r,1);
  k=q.find_del(stu_c,stu_r,1);

  }
*/
#endif
