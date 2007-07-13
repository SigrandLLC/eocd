#ifndef EOC_POLLER_REQUESTS_H
#define EOC_POLLER_REQUESTS_H

#include <generic/EOC_requests.h>
#include <generic/EOC_msg.h>
#include <db/EOC_db.h>
#include <engine/EOC_handlers.h>

EOC_msg *_req_discovery(sched_state stat,sched_elem el,EOC_config *cfg);
EOC_msg *_req_inventory(sched_state stat,sched_elem el,EOC_config *cfg);
EOC_msg *_req_configure(sched_state stat,sched_elem el,EOC_config *cfg);
EOC_msg *_req_test(sched_state stat,sched_elem el,EOC_config *cfg);

#endif
