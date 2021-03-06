#include <app-if/app_frame.h>

app_frame::
app_frame(app_ids id,app_types type,roles role,u8 act_seconds,char *dname){
	u32 psize,csize;
	int offs;

	if( (offs = size_by_id(id,type,psize,csize) ) <0 ){
	    buf = NULL;
	    buf_size = 0;
	    return;
	}
	buf_size = offs;
	if( !(buf = new char[buf_size]) ){
		buf = NULL;
	    buf_size = 0;
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
	if( time(&hdr->tstamp) < 0){
		//   PDEBUG(DERR,"Error getting current time");
	}
	hdr->act_sec = act_seconds;
	memcpy(hdr->dname,dname,strnlen(dname,SPAN_NAME_LEN));
}

app_frame::
app_frame(char *b,int size){
    u32 psize,csize;
    int offs;
    buf = b;
    buf_size = size;
    hdr = (app_frame_hdr *)buf;
    // GET correct parameters of frame
    if( (offs = size_by_id((app_ids)hdr->id,(app_types)hdr->type,psize,csize) ) <0 ){
        buf = NULL;
        buf_size = 0;
        PDEBUG(DERR,"Cannot get info about frame id = %d",hdr->id);
        return;
    }
    if( (hdr->psize != psize) || (hdr->csize != csize) ){
        PDEBUG(DERR,"Error in app_frame header: id=%d, psize=(%d not %d), csize=(%d not %d)",
			   hdr->id,hdr->psize,psize,hdr->csize,csize);
		negative();
    }
}

app_frame::
~app_frame(){
    if(buf)
        delete[] buf;
}


int app_frame::
size_by_id(app_ids id,app_types type,u32 &psize,u32 &csize)
{
    int size = FRAME_HEADER_SZ;

    switch(id){
    case APP_SPAN_NAME:
        psize = SPAN_NAME_PAY_SZ;
        csize = SPAN_NAME_CH_SZ;
        break;
    case APP_SPAN_PARAMS:
        psize = SPAN_PARAMS_PAY_SZ;
        csize = SPAN_PARAMS_CH_SZ;
        break;
    case APP_SPAN_CONF:
        psize = SPAN_CONF_PAY_SZ;
        csize = SPAN_CONF_CH_SZ;
        break;
    case APP_SPAN_STATUS:
        psize = SPAN_STATUS_PAY_SZ;
        csize = SPAN_STATUS_CH_SZ;
        break;
    case APP_INVENTORY:
        psize = INVENTORY_PAY_SZ;
        csize = INVENTORY_CH_SZ;
        break;
    case APP_ENDP_CONF:
        psize = ENDP_CONF_PAY_SZ;
        csize = ENDP_CONF_CH_SZ;
        break;
    case APP_ENDP_CUR:
        psize = ENDP_CUR_PAY_SZ;
        csize = ENDP_CUR_CH_SZ;
        break;
    case APP_ENDP_15MIN:
        psize = ENDP_15MIN_PAY_SZ;
        csize = ENDP_15MIN_CH_SZ;
        break;
    case APP_ENDP_1DAY:
        psize = ENDP_1DAY_PAY_SZ;
        csize = ENDP_1DAY_CH_SZ;
        break;
    case APP_ENDP_MAINT:
        psize = ENDP_MAINT_PAY_SZ;
        csize = ENDP_MAINT_CH_SZ;
        break;
    case APP_CPROF:
		psize = CPROF_PAY_SZ;
		csize = CPROF_CH_SZ;
		break;
    case APP_LIST_CPROF:
		psize = CPROF_LIST_PAY_SZ;
		csize = CPROF_LIST_CH_SZ;
		break;
    case APP_ADD_CPROF:
		psize = CPROF_ADD_PAY_SZ;
		csize = CPROF_ADD_CH_SZ;
		break;
    case APP_DEL_CPROF:
		psize = CPROF_DEL_PAY_SZ;
		csize = CPROF_DEL_CH_SZ;
		break;
    case APP_ADD_CHAN:
		psize = CHAN_ADD_PAY_SZ;
		csize = CHAN_ADD_CH_SZ;
		break;
    case APP_DEL_CHAN:
		psize = CHAN_DEL_PAY_SZ;
		csize = CHAN_DEL_CH_SZ;
		break;
    case APP_CHNG_CHAN:
		psize = CHAN_CHNG_PAY_SZ;
		csize = CHAN_CHNG_CH_SZ;
		break;
    case APP_LOOP_RCNTRST:
		psize = LOOP_RCNTRST_PAY_SZ;
		csize = LOOP_RCNTRST_CH_SZ;
		break;
    case APP_DUMP_CFG:
		psize = DUMP_CFG_PAY_SZ;
		csize = DUMP_CFG_CH_SZ;
		break;
	case APP_SENSORS:
		psize = SENSORS_PAY_SZ;
		csize = SENSORS_CH_SZ;
		break;
	case APP_SENSOR_FULL:
		psize = SENSOR_FULL_PAY_SZ;
		csize = SENSOR_FULL_CH_SZ;
		break;
    default:
        return -1;
    }

    size += psize;
    if( type == APP_SET ){
        size += csize;
    }else{
        csize = 0;
    }
    return size;
}

const char *app_frame::
chan_name(){
	return hdr->dname;
}

void app_frame::
chan_name(char *name)
{
    strncpy(hdr->dname,name,SPAN_NAME_LEN);
}

char *app_frame::
payload_ptr(){
    ASSERT( buf );
    if( buf )
		return &buf[FRAME_HEADER_SZ];
    return NULL;
}

char * app_frame::
changelist_ptr(){
    ASSERT(buf && hdr->csize);
    if( buf && hdr->csize )
        return &buf[FRAME_HEADER_SZ + hdr->psize];
    return NULL;
}

