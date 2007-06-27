#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <stdlib.h>

EOC_msg::EOC_msg(){
	buf = NULL;
	size = 0;
	dir = UNDEFINED;
}

EOC_msg::~EOC_msg(){
	if( buf )
	    free(buf);
}

//---- Get/Set direction ----//
void
EOC_msg::direction( Direction d){
    dir = d;
}
EOC_msg::Direction
EOC_msg::direction(){
    return dir;
}

//---- Get/Set message type ----//
int
EOC_msg::type(){
    if( !buf || size < 2)
        return -1;
    return buf[1];
}

int
EOC_msg::type(unsigned char t){
    if( !buf || size < 2)
        return -1;
    buf[1] = t;
    return 0;
}

//---- Get address information ----//
unit
EOC_msg::dst(){
    return (unit)(buf[0]&0xf);
}

unit
EOC_msg::src(){
    return (unit)((buf[0]&0xf0)>>4);
}

//---- Set address information ----//
int
EOC_msg::dst(unit dst){
    if( (dst&0xf) != dst )
	return -1;
    buf[0] &= 0xf0;	
    buf[0] |= dst&0xf;
    return 0;
}

int
EOC_msg::src(unit src){
    if( (src&0xf) != src )
	return -1;
    buf[0] &= 0x0f;
    buf[0] |= (src&0xf)<<4;
    return 0;
}

//---- initialize EOC message ----//
int
EOC_msg::setup(char *ptr,int sz)
{
    if( buf )
	return -1;
    buf = ptr;
    size = sz;	
    // check for correctness
    if( (dst() < stu_c || dst() > sru8) ||
	    (src() < stu_c || src() > sru8) ){
	buf = NULL;
	sz = 0;
	return -1;
    }
    return 0;
}

void
EOC_msg::clean()
{
    if( !buf )
	return;
    free(buf);
    size = 0;
    buf = NULL;    
    dir = UNDEFINED;
}

int
EOC_msg::response(int sz)
{
    unit d = dst();
    unit s = src();
    int id = type();
    if( id > 127 || dir == UNDEFINED )
	return -1;
    if( size < sz+EOC_HEADER ){
    	free(buf);
    	if( !(buf = (char*)malloc(sz+EOC_HEADER)) )
	    return -1;
    }
    size = sz+EOC_HEADER;    
    dst(s);
    src(d);
    type(id+128);
    dir = (dir == DOWNSTREAM) ? UPSTREAM : DOWNSTREAM;
    return 0;
}
