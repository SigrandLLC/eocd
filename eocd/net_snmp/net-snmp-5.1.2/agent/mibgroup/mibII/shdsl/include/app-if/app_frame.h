#ifndef APP_FRAME_H
#define APP_FRAME_H

#include <stdio.h>
#include <generic/EOC_types.h>
#include <eoc_debug.h>
#include <string.h>
#include <time.h>

#include <app-if/app_messages.h>

#define APP_FRAME_DEBUG 0

class app_frame{
public:
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
	char dname[SPAN_NAME_LEN];
	time_t tstamp;
	u8 act_sec;
    }app_frame_hdr;
    
    app_frame_hdr *hdr;
    enum {FRAME_HEADER_SZ = sizeof(app_frame_hdr) };
    char *buf;
    u32 buf_size;
    int size_by_id(app_ids id,app_types type,u32 &psize,u32 &csize);
    
public:
    app_frame(app_ids id,app_types type,roles role,u8 act_sec,char *dname = "\0");
    app_frame(char *b,int size);
    ~app_frame();
    const char *chan_name();
    void chan_name(char *name);
    char *payload_ptr();
    char *changelist_ptr();
    char *frame_ptr(){ return buf; }
    int frame_size(){ return buf_size; }
    app_ids id(){ return (app_ids)hdr->id; }
    app_types type(){ return (app_types)hdr->type; }
    roles role(){ return (roles)hdr->role; }
    void negative(){ hdr->error = 1; }
    int is_negative(){ return hdr->error; }
    void response(){ hdr->role = RESPONSE; }
    int info_uptodate(){
	time_t cur;
	if(time(&cur) < 0){
	    PDEBUG(DERR,"Error getting current time");
	}
	return ((cur-hdr->tstamp)<hdr->act_sec) ? 1 : 0;
    }
	
};

#endif
