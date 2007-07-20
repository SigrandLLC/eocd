#ifndef EOC_SIDE_H
#define EOC_SIDE_H

#include <eoc_debug.h>

class EOC_side{
public:

private:
    EOC_loop *loops[MAX_LOOPS];
    int loop_num;
public:
    EOC_side(int lnum){
	ASSERT( lnum < MAX_LOOPS );
	loop_num = lnum;
	for(int i=0;i<lnum;i++){
	    loops[i] = new EOC_loop;
	}
    }

    ~EOC_side(){
	for(int i=0;i<loop_num;i++){
	    ASSERT( loops[i] );
	    delete loops[i];
	}
    }
    
    EOC_loop* get_loop(int l){
	if( !(l<loop_num) ){
	    return NULL;
	}
	return loops[l];
    }
    
};

#endif
