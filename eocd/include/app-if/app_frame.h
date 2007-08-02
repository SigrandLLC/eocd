#ifndef APP_FRAME_H
#define APP_FRAME_H

#include <stdio.h>
#include <generic/EOC_types.h>
#include <eocd_log.h>
#include <eoc_debug.h>
#include <string.h>

#include <app-if/app_messages.h>

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
    int size_by_id(ids id,types type,u32 &psize,u32 &csize);
    
public:
    app_frame(ids id,types type,roles role,char *dname = "\0");
    app_frame(char *b,int size);
    ~app_frame();
    const char *chan_name();
    char *payload_ptr();
    char *changelist_ptr();
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
