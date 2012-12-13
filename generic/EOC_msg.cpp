#include <generic/EOC_generic.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <generic/EOC_msg.h>
#include <stdlib.h>

EOC_msg::EOC_msg(){
    buf = NULL;
    bsize = 0;
    size = 0;
    dir = NOSTREAM;
}

EOC_msg::EOC_msg(int sz){
    dir = NOSTREAM;
    if( !(buf = (char*)malloc(sz + EOC_HEADER)) ){
		bsize = 0;
		size = 0;
		return;
    }
    bsize = sz+EOC_HEADER;
    size = bsize;
}

EOC_msg::EOC_msg(EOC_msg *ex)
{
    size = ex->msize();
    bsize = ex->msize();
    dir = ex->direction();
    buf = (char*)malloc(bsize);
    if( buf )
		memcpy(buf,ex->mptr(),size);
    else{
		bsize = 0;
		size =0;
    }

}

EOC_msg::EOC_msg(EOC_msg *ex,int new_size)
{
    bsize = new_size+EOC_HEADER;
    size = bsize;
    dir = ex->direction();
    buf = (char*)malloc(bsize);
    if( buf )
		memcpy(buf,ex->mptr(),size);
    else{
		// TODO: EXCEPTION!!!
		bsize = 0;
		size =0;
    }
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
unsigned char
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
    if( buf )
		return (unit)(buf[0]&0xf);
    return unknown;
}

unit
EOC_msg::src(){
    if( buf )
        return (unit)((buf[0]&0xf0)>>4);
    return unknown;
}

//---- Set address information ----//
int
EOC_msg::dst(unit dst){
    if( (dst&0xf) != dst || !buf  )
		return -1;
    buf[0] &= 0xf0;
    buf[0] |= dst&0xf;
    return 0;
}

int
EOC_msg::src(unit src){
    if( (src&0xf) != src || !buf  )
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
    bsize = sz;
    // check for correctness
    if( (dst() < unknown || (dst() > sru8 && dst()!=BCAST) ) ||
	    (src() < unknown || (src() > sru8 && src()!=BCAST) ) ){
		buf = NULL;
		size = 0;
		bsize = 0;
		return -1;
    }
    return 0;
}

void
EOC_msg::clean()
{
    if( buf )
		free(buf);
    size = 0;
    buf = NULL;
    dir = NOSTREAM;
}


int
EOC_msg::resize(int sz)
{
    if( bsize < sz+EOC_HEADER ){
		if( buf )
			free(buf);
    	if( !(buf = (char*)malloc(sz+EOC_HEADER)) ){
			bsize = 0;
			size =0;
			return -1;
		}
		bsize = sz+EOC_HEADER;
    }
    size = sz+EOC_HEADER;
}


int
EOC_msg::response(int sz)
{
    unit d = dst();
    unit s = src();
    int id = type();
    if( id > 127 || dir == NOSTREAM )
		return -1;
    resize(sz);
    dst(s);
    src(d);
    type(id+RESP_OFFSET);
    dir = (dir == DOWNSTREAM) ? UPSTREAM : DOWNSTREAM;
    return 0;
}
