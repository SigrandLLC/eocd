#ifndef EOC_MSG_H
#define EOC_MSG_H

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
	char *chname;
    enum Direction dir;
public:
    EOC_msg();
    EOC_msg(int size);
    EOC_msg(EOC_msg *ex);
    EOC_msg(EOC_msg *ex,int new_size);
    ~EOC_msg();
    void direction(enum Direction d);
    Direction direction();
    unsigned char type();
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
	inline void set_chname(char *n){ chname = n; }
	inline char *get_chname(){ return chname; }

    // class of message
    inline int is_request(){
	if( type() < 127 )
	    return 1;
	return 0;
    }
    inline int is_response(){ return !is_request(); }

};
#endif

