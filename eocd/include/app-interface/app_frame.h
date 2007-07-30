#ifndef APP_FRAME_H
#define APP_FRAME_H

#include <stdio.h>
#include <generic/EOC_types.h>
#include <eocd_log.h>
#include <eoc_debug.h>
#include <string.h>

#include <app-interface/app_messages.h>

#define APP_FRAME_DEBUG 0

class app_frame{
public:
    typedef enum { SPAN_CONF=0,SPAN_STATUS,INVENTORY,ENDP_CONF,ENDP_CUR,
	      ENDP_15MIN,ENDP_1DAY,ENDP_MAINT,UNIT_MAINT,
	      SPAN_CONF_PROF,ENDP_ALARM_PROF} ids;
    static const int ids_num = 11;
    typedef enum { SET,GET,GET_NEXT } types;
    typedef enum { REQUEST,RESPONSE } roles;

protected:
    typedef struct {
	u32 len;
	u32 psize;
	u16 csize;
	u8 id;
	u8 type:2;
	u8 role:1;
	u8 error;
	u8 dname_offs;
    }app_frame_hdr;
    
    app_frame_hdr *hdr;
    enum {FRAME_HEADER_SZ = sizeof(app_frame_hdr) };
    char *buf;
    u32 buf_size;

    int size_by_id(ids id,types type,u32 &psize,u32 &csize)
    {
	int size = FRAME_HEADER_SZ;

	switch(id){
	case SPAN_CONF:
	    psize = SPAN_CONF_PAY_SZ;
	    csize = SPAN_CONF_CH_SZ;
	    break;
	case SPAN_STATUS:
	    psize = SPAN_STATUS_PAY_SZ;
	    csize = SPAN_STATUS_CH_SZ;
	    break;
	case INVENTORY:
	    psize = INVENTORY_PAY_SZ;
	    csize = INVENTORY_CH_SZ;
	    break;
	case ENDP_CONF:
	    psize = ENDP_CONF_PAY_SZ;
	    csize = ENDP_CONF_CH_SZ;
	    break;
	case ENDP_CUR:
	    psize = ENDP_CUR_PAY_SZ;
	    csize = ENDP_CUR_CH_SZ;
	    break;
	case ENDP_15MIN:
	    psize = ENDP_15MIN_PAY_SZ;
	    csize = ENDP_15MIN_CH_SZ;
	    break;
	case ENDP_1DAY:
	    psize = ENDP_1DAY_PAY_SZ;
	    csize = ENDP_1DAY_CH_SZ;
	    break;
	case ENDP_MAINT:
	    psize = ENDP_MAINT_PAY_SZ;
	    csize = ENDP_MAINT_CH_SZ;
	    break;
	default:
	    return -1;
	}

	size += psize;
	if( type == SET ){
	    size += csize;
	}else{
	    csize = 0;
	}
	return size;    
    }
    
public:
    app_frame(ids id,types type,roles role,char *dname = "\0"){
	u32 psize,csize;
	int offs;
	if( (offs = size_by_id(id,type,psize,csize) ) <0 ){
	    buf = NULL;
	    buf_size = 0;
	    return;
	}
	buf_size = offs + strnlen(dname,256)+1;
	if( !(buf = new char[buf_size]) ){
	    buf_size = 0;
	    eocd_log(0,"Not enought memory");
	    return;
	}
	memset(buf,0,buf_size);
	hdr = (app_frame_hdr *)buf;
	hdr->len = buf_size;
	hdr->psize = psize;
	hdr->csize = csize;
	hdr->id = (u8)id;
	hdr->type = (u8)type;
	hdr->role = (u8)role;
	hdr->dname_offs = offs;
	memcpy(buf+offs,dname,strnlen(dname,256));
    }

    app_frame(char *b,int size){
	u32 psize,csize;
	int offs;
	buf = b;
	hdr = (app_frame_hdr *)buf;
	// GET correct parameters of frame
	if( (offs = size_by_id((ids)hdr->id,(types)hdr->type,psize,csize) ) <0 ){
	    buf = NULL;
	    size = 0;
	    eocd_log(0,"Cannot get info about frame id = %d",hdr->id);
	    return;
	}
	if( (hdr->psize != psize) || (hdr->csize != csize) ||
	    (hdr->dname_offs != offs) || (!csize && hdr->type == SET) ){
	    eocd_log(0,"Error in app_frame header");
	    buf = NULL;
	    size = 0;
	}
    }


    ~app_frame(){
	ASSERT(buf);
	if(buf)
	    delete[] buf;
    }
    
    
    const char *chan_name(){
	return &buf[hdr->dname_offs];
    }


    char *payload_ptr(){
	ASSERT(buf);
	if( buf )
	    return &buf[FRAME_HEADER_SZ];
	return NULL;
    }    

    char *changelist_ptr(){
	ASSERT(buf && hdr->csize);
	if( buf && hdr->csize )
	    return &buf[FRAME_HEADER_SZ + hdr->psize];
	return NULL;
    }
    char *frame_ptr(){ return buf; }
    int frame_size(){ return buf_size; }
    
    ids id(){ return (ids)hdr->id; }
    types type(){ return (types)hdr->type; }
    roles role(){ return (roles)hdr->role; }
    void negative(){ hdr->error = 1; }
    int is_negative(){ return hdr->error; }
    void response(){ hdr->role = RESPONSE; }
};

#endif
