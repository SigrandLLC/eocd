#include <stdio.h>
#include "include/db/eoc_db.h"
#include "include/utils/hash_table.h"
aaaaaaaaaaaa
//--------- Allocate New Database item -------------//
db_t *
db_alloc()
{
    db_t *db = malloc( sizeof(db_t) );
    memset(db,0,sizeof(db_t));
    return db;
}

//--------- Add/get span to/from DB --------------------//
int
db_add_span(db_t *db)
{
    if( !db ){
	eocd_log(ERROR,"cannot work with NULL db!");
	return -1;
    }
    if( !(db->span_num+1 < MAX_SPAN_NUM) ){
	eocd_log(WARNING,"Limit of spans exceded");
	return -1;
    }
    db->spans[db->span_num] = span_alloc();
    db->span_num++;
}

db_t *
db_get_span(db_t *db,int num)
{
    if( !(num < db->span_num) )
	return NULL;
    return db->spans[num];
}

//-------- Span configuration manipulation ----------//

int
db_span_conf_set(db_t *db,int num,char *name)
{
    if( !(num < db->span_num) )
	return -EARG;
    if( htable_find(&db->conf_tbl,name) == NULL )
	return -ENOENT;
    db->spans[num]->conf = strdup(name);    
    return 0;
}
conf_prof_t *
db_span_conf_get(db_t *db,int num)
{
    if( !(num < db->span_num) )
	return -1;
    return (conf_prof_t *)htable_find(&db->conf_tbl,db->spans[num]->conf);
}

int
db_span_alarm_set(db_t *db,int num,char *name)
{
    if( !(num < db->span_num) )
	return -EARG;
    if( htable_find(&db->alarm_tbl,name) == NULL )
	return -ENOENT;
    db->spans[num]->alarm = strdup(name);    
    return 0;
}

alarm_prof_t *
db_span_alarm_get(db_t *db,int num)
{
    if( !(num < db->span_num) )
	return -1;
    return (alarm_prof_t *)htable_find(&db->alarm_tbl,db->spans[num]->alarm);
}

//--------- Profiles manipulation functions -----------//

// Configuration profile

inline int
db_add_conf(db_t *db,char *name,void *data){
    return htable_add(&db->conf_tbl,name,data);
}

inline conf_prof_t *
db_get_conf_data(db_t *db,char *name){
    return (conf_prof_t *)htable_find(&db->conf_tbl,name);
}

inline conf_prof_t *
db_del_conf(db_t *db,char *del){
    return (conf_prof_t *)htable_del(&db->conf_tbl,name);
}

// Alarm profile
inline int
db_add_conf(db_t *db,char *name,void *data){
    return htable_add(&db->alarm_tbl,name,data);
}

inline alarm_prof_t *
db_get_conf_data(db_t *db,char *name){
    return (alarm_prof_t *)htable_find(&db->alarm_tbl,name);
}

inline alarm_prof_t *
db_del_conf(db_t *db,char *del){
    return (alarm_prof_t *)htable_del(&db->alarm_tbl,name);
}
