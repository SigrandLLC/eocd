#include <engine/responds.h>
#include <generic/EOC_msg.h>
#include <engine/EOC_db.h>

int discovery(EOC_db *db,EOC_msg *m)
{
    if( !m || !db )
	return -1;
    if( m->type() != RESP_DISCOVERY )
	return -1;
    
    unit cur = m->src();
    db->add_unit(cur);
}