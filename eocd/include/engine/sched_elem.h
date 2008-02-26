#ifndef SCHED_ELEM_H
#define SCHED_ELEM_H

#include <generic/EOC_generic.h>
#include <list>

using namespace std;

class __timestamp{
 protected:
    unsigned long ticks;
 public:
    __timestamp(){ ticks = 0; }
    __timestamp(unsigned int t){ ticks = t; }
    __timestamp(const __timestamp &t){ ticks = t.ticks; }
    __timestamp(__timestamp t,unsigned int offs){ ticks = t.ticks + offs; }

    inline __timestamp & operator =(__timestamp &right){
	ticks = right.ticks;
	return *this;
}

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
    inline __timestamp &operator+(__timestamp &right){
		__timestamp *n = new __timestamp(*this);
		n->ticks += right.ticks;
		return *n;
    }

    inline __timestamp operator+(int &offs){
		__timestamp n = *this;
		n.ticks += offs;
		return n;
    }

    inline int operator-(__timestamp &right){
		return (this->ticks - right.ticks);
    }

    int get_val(){ return ticks; }

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
    inline sched_elem &operator = (sched_elem &right){
	src = right.src;
	dst = right.dst;
	type = right.type;
	tstamp = right.tstamp;
	return *this;
}
};

#endif
