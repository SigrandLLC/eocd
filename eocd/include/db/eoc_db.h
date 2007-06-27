#ifndef SIGRAND_EOC_DB_H
#define SIGRAND_EOC_DB_H

#include "eoc_primitives.h"
#include "span.h"
#include "../utils/span.h"
#include "conf_profile.h"
#include "alarm_profile.h"

#define MAX_SPAN_NUM 20
#define MAX_PROF_NUM 40

typedef  struct{
    eocd_span_t *spans[MAX_SPAN_NUM];
    int span_num;
    hash_table_t *conf_tbl;
    hash_table_t *alarm_tbl;    
} eocd_db_t;

db_t *db_alloc();
//--------- Add/get span to/from DB --------------------//
int db_add_span(db_t *db);
db_t *db_get_span(db_t *db,int num);
//-------- Span configuration manipulation ----------//
int db_span_conf_set(db_t *db,int num,char *name);
conf_prof_t *db_span_conf_get(db_t *db,int num);
int db_span_alarm_set(db_t *db,int num,char *name);
alarm_prof_t *db_span_alarm_get(db_t *db,int num);

//--------- Profiles manipulation functions -----------//
// Configuration profile
inline int db_add_conf(db_t *db,char *name,void *data);
inline conf_prof_t *db_get_conf_data(db_t *db,char *name);
inline conf_prof_t *db_del_conf(db_t *db,char *del);
// Alarm profile
inline int db_add_conf(db_t *db,char *name,void *data);
inline alarm_prof_t *db_get_conf_data(db_t *db,char *name);
inline alarm_prof_t *db_del_conf(db_t *db,char *del);

#endif
