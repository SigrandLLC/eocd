#ifndef EOC_SIDE_H
#define EOC_SIDE_H

#include <db/EOC_loop.h>
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
	if( !(l<loop_num && l>=0) ){
	    PDEBUG(0,"error loop number = %d",l);
	    return NULL;
	}
	return loops[l];
    }
	inline void link_up(){
		for(int i=0;i<loop_num;i++)
			if(loops[i])
				loops[i]->link_up();
	}
	inline void link_down(){
		PDEBUG(DERR,"side func");
		for(int i=0;i<loop_num;i++){
			if(loops[i]){
				PDEBUG(DERR,"try down loop#%d",i);
				loops[i]->link_down();
				PDEBUG(DERR,"try down loop#%d - success",i);
			}
		}
	}
};

#endif
