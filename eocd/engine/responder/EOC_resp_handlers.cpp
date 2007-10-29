/*
 * EOC_resp_handlers.cpp:
 *	Contains handlers for EOC requests on Regenerators (SRUx) 
 *	& Terminators (STU-C,STU-R)
 *	Prototype:
 *		EOC_msg *(*responder_handler_t)(EOC_dev *dev,EOC_msg *m);
 */  

#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/EOC_handlers.h>
#include <engine/EOC_router.h>


int
EOC_responder::_inventory(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    ASSERT( in && m );
    ASSERT( m->type() == REQ_INVENTORY );
    ret = NULL;
    cnt = 1;
    m->response(RESP_INVENTORY_SZ);
    resp_inventory *resp = (resp_inventory *)m->payload();
    memset(resp,0,RESP_INVENTORY_SZ);
    resp->shdsl_ver = 0x08;
    strncpy((char*)resp->ven_lst,"001",4); // Hardware version
    strncpy((char*)resp->ven_issue,"01",3); // Usage of the unit
    strncpy((char*)resp->softw_ver,"000001",7); // Software version 
    memset(resp->unit_id_code,0,sizeof(resp->unit_id_code));
    strncpy((char*)resp->ven_id,"Sgr\0",9);    
    strncpy((char*)resp->ven_model,"001",14);
    strncpy((char*)resp->ven_serial,"001",14);
    memset((char*)resp->other,0,sizeof(resp->other));

    return 0;
}

int
EOC_responder::_configure(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    ASSERT( in && m );
    ASSERT(m->type() == REQ_CONFIGURE );
    EOC_router *r = in->r;
    req_configure *req = (req_configure *)m->payload();
    u8 normal = req->conf_type;
    u8 loop = req->loop_attn;
    u8 snr = req->snr_marg;
    cnt = 1;
    ret = NULL;
    m->response(RESP_CONFIGURE_SZ);
    resp_configure *resp = (resp_configure *)m->payload();
    memset(resp,0,RESP_CONFIGURE_SZ);
    if( normal ){
		if( r->csdev() && r->csdev()->tresholds(loop,snr) )
			resp->utc = 1;
		if( r->nsdev() && r->nsdev()->tresholds(loop,snr) )
			resp->utc = 1;
    }
    resp->loop_attn = loop;
    resp->snr_marg = snr; 
    return 0;
}

int
collect_statistic(EOC_dev *dev,int loop_num,side_perf *perf,int *perf_change)
{
    int ch_counter = 0;
    for(int loop=0;loop<loop_num;loop++){
        int k=-1,ret;
        do{
    	    ret = dev->statistics(loop,perf[loop]);
			PDEBUG(DFULL,"Dev return nonzero statistic");
			k++;
		}while( (ret < 0) && (k<3) );
	
		if( ret<0 )
	    	return -1;
	
		if( !ret ){
			perf_change[loop] = 0;
			continue;
		}
		perf_change[loop] = 1;
		ch_counter++;
    }
    return ch_counter;
}


int
EOC_responder::_status(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    ASSERT( in && m );
    ASSERT(m->type() == REQ_STATUS );
    EOC_router *r = in->r;
    EOC_dev *ns = r->nsdev();
    EOC_dev *cs = r->csdev();
    int loop_num = r->loops();
    int loop,offs=0;
    side_perf ns_perf[loop_num],cs_perf[loop_num];
    int ns_perf_ch[loop_num],cs_perf_ch[loop_num];
    int cs_loops_ch = 0,ns_loops_ch = 0;
    EOC_msg **array;

    // accumulate statistics about all presented sides
    memset(ns_perf,0,sizeof(ns_perf));
    memset(cs_perf,0,sizeof(cs_perf));

    if( cs ){
		if( (cs_loops_ch = collect_statistic(cs,loop_num,cs_perf,cs_perf_ch)) < 0 )
			return -1;
    }
    if( ns ){
		if( (ns_loops_ch = collect_statistic(ns,loop_num,ns_perf,ns_perf_ch)) < 0 )
			return -1;
    }
    
    int array_len = loop_num + cs_loops_ch + ns_loops_ch;
    array = new EOC_msg*[array_len];
    for(int i=0;i<array_len;i++){
		array[i] = NULL;
    }

    // Generate status responses
    m->response(RESP_STATUS_SZ);
    for(loop=0;loop<loop_num;loop++){
		array[loop] = new EOC_msg(m);
		resp_status *resp = (resp_status *)array[loop+offs]->payload();
		memset(resp,0,RESP_STATUS_SZ);
		resp->ns_snr_marg = ns_perf[loop].snr_marg;
		resp->cs_snr_marg = cs_perf[loop].snr_marg;
		resp->loop_id = loop+1;
    }
    offs += loop;
    // Setup
    if( cs ){
		for(loop=0;loop<loop_num;loop++){
			if( cs_perf_ch[loop] ){
				if( !(array[offs] = new EOC_msg(m,SIDE_PERF_SZ)) )
					goto err_exit;
				EOC_msg *t = array[offs];
				t->type(RESP_CSIDE_PERF);
				offs++;
				*(side_perf*)t->payload() = cs_perf[loop];
				((side_perf*)t->payload())->loop_id = loop+1;	 
			}
		}
    }	    
    // network side
    if( ns ){
		for(loop=0;loop<loop_num;loop++){
			if( ns_perf_ch[loop] ){
				if( !(array[offs] = new EOC_msg(m,SIDE_PERF_SZ)) )
					goto err_exit;
				EOC_msg *t = array[offs];
				t->type(RESP_NSIDE_PERF);
				offs++;
				*(side_perf*)t->payload() = ns_perf[loop]; 
				((side_perf*)t->payload())->loop_id = loop+1;	 
			}
		}
    }	    

#ifdef REPEATER
    extern u8 sensor_alarm_1,sensor_alarm_2,sensor_alarm_3;
	if( sensor_alarm_1 || sensor_alarm_2 || sensor_alarm_3 ){
		if( !(array[offs] = new EOC_msg(m,RESP_SENSOR_STATE_SZ)) )
            goto err_exit;
		EOC_msg *t = array[offs];
		t->type(RESP_SENSOR_STATE);
		offs++;
		resp_sensor_state *resp = (resp_sensor_state *)t->payload();
		resp->sensor1 = sensor_alarm_1;
		resp->sensor2 = sensor_alarm_2;
		resp->sensor3 = sensor_alarm_3;
	 
		sensor_alarm_1 = 0;
		sensor_alarm_2 = 0;
		sensor_alarm_3 = 0;
	}
#endif

    cnt = offs;
    ret = array;
    return 0;
 err_exit:
    for(int i=0;i<offs;i++){
		if( array[i] )
			delete array[i];
    }
    delete[] array;
    ret = NULL;
    return -1;
}


int
EOC_responder::_test(EOC_responder *in,EOC_msg *m,EOC_msg **&ret,int &cnt)
{
    if( m->type() != 15 )
		return -1;
    printf("TEST_REQUEST getted\n");
    m->response(10);
    return 0;
}

