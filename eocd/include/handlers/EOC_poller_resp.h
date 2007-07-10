#ifndef EOC_POLLER_RESPONSES_H
#define EOC_POLLER_RESPONSES_H

#include <generic/EOC_responses.h>
#include <generic/EOC_msg.h>
#include <db/EOC_db.h>

int _resp_discovery(EOC_db *db,EOC_msg *m);
int _resp_inventory(EOC_db *db,EOC_msg *m);
int _resp_configure(EOC_db *db,EOC_msg *m);

#endif
