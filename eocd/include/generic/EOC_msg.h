#ifndef SIGRAND_EOC_MSG_H
#define SIGRAND_EOC_MSG_H

#include <stdio.h>
#include <string.h>

#include <generic/EOC_generic.h>

class EOC_msg{
public:
    enum Direction { NOSTREAM,DOWNSTREAM, UPSTREAM };
    enum EOC_CONSTS {EOC_HEADER=2};
protected:
    char *buf;
    int size,bsize;
    enum Direction dir;
public:
    EOC_msg();
    EOC_msg(int size);    
    EOC_msg(EOC_msg *ex);
    EOC_msg(EOC_msg *ex,int new_size);    
    ~EOC_msg();
    void direction(enum Direction d);
    Direction direction();
    int type();
    int type(unsigned char);
    unit dst();
    int dst(unit dst);
    unit src();
    int src(unit src);
    int setup(char *ptr,int size);
    void clean();
    
    inline char *mptr(){ return buf; }
    inline int msize(){	return size; }
    inline char *payload(){ return &buf[2]; }
    inline int payload_sz(){ return size-2; }    
    int response(int);
    int resize(int sz);
    
    // class of message
    inline int is_response(){
	if( type() < 127 )
	    return 1;
	return 0;
    }
    inline int is_request(){ return !is_response(); }
	
};
#endif

