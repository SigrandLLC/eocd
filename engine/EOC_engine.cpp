#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/EOC_router.h>
#include <engine/EOC_responder.h>
#include <engine/EOC_poller.h>
#include <engine/EOC_engine.h>
#include <handlers/EOC_poller_req.h>
#include <eoc_debug.h>
#include <syslog.h>

// Terminal constructor
EOC_engine::EOC_engine(EOC_dev_terminal *d1, EOC_config *c,dev_type t,EOC_dev::dev_del_func df,
	u16 rmax) {
	PDEBUG(DFULL,"start");
	ASSERT(d1 && (t == master || t == slave) && c);
	cfg = c;
	type = t;
	recv_max = rmax;
	PDEBUG(DFULL,"create EOC_router");
	rtr = new EOC_router(type, d1, df);
	PDEBUG(DFULL,"create EOC_responder");
	resp = new EOC_responder(rtr);
	dev1 = d1;
	dev2 = NULL;
	PDEBUG(DFULL,"Finish");
}

//----------------------------------------------------------------------
// Repeater constructor
EOC_engine::EOC_engine(EOC_dev *d1, EOC_dev *d2,EOC_dev::dev_del_func df,u16 rmax) {
	ASSERT(d1 && d2);
	type = repeater;
	recv_max = rmax;
	rtr = new EOC_router(type, d1, d2, df);
	resp = new EOC_responder(rtr);
	dev1 = d1;
	dev2 = d2;
}

//----------------------------------------------------------------------
// setup_state - setups poller link status if i really changes
// on device

int EOC_engine::setup_state() {
	ASSERT(rtr && resp); // error in constructor
	rtr->update_state();
	return 0;
}

//----------------------------------------------------------------------
// schedule - call it to take control to engine to process incoming and
// outgoing messages
int EOC_engine::schedule(char *ch_name) {
	static int number = -1;
	EOC_msg *m, **ret;
	ASSERT(rtr && resp); // Constructor failed

	int i = 0;
	int cnt;
	number++;
	PDEBUG(DFULL, "%d schedule started\n", number);

	while ((m = rtr->receive()) && i < recv_max) {
		PDEBUG(DFULL, "%d schedule: message: src(%d) dst(%d) id(%d)\n", number,
			m->src(), m->dst(), m->type());
		if (m->is_request()) {
			if (resp->request(m, ret, cnt)) {
				delete m;
				return -1;
			}
			if (!ret) {
				// only one message to respond
				if (rtr->send(m)) {
					delete m;
					return -1;
				}
			} else if (ret) {
				// several messages to respond
				for (i = 0; i < cnt; i++) {
					if (rtr->send(ret[i])) {
						for (int j = 0; j < cnt; j++) {
							delete ret[j];
						}
						delete[] ret;
						delete m;
						return -1;
					}
				}
				for (int j = 0; j < cnt; j++) {
					delete ret[j];
				}
				delete[] ret;
			}
		}
		delete m;
		i++;
	}
	return 0;
}

int EOC_engine::configure(char *ch_name) {
	EOC_dev_terminal *dev;
	int ret = 0;

	PDEBUG(DINFO, "start");
	switch (type) {
	case master:
		dev = (EOC_dev_terminal*) rtr->csdev();
		break;
	case slave:
		dev = (EOC_dev_terminal*) rtr->nsdev();
		break;
	default:
		return 0;
	}
	PDEBUG(DERR, "dev=%p", dev);
	if (!dev) {
		PDEBUG(DERR, "(%s): Error router initialisation", ch_name);
		return -1;
	}
	PDEBUG(DERR, "Get conf profile, cfg=%p", cfg);
	conf_profile *prof = (conf_profile *) cfg->conf();
	PDEBUG(DERR, "cprof=%s", prof->name);
	if (!prof) {
		syslog(LOG_ERR,"Profile %s not exist. Try revert to old.", cfg->cprof());
		PDEBUG(DERR, "Profile %s not exist. Try revert to old.", cfg->cprof());
		cfg->cprof_revert();
		prof = (conf_profile *) cfg->conf();
		if (!prof) {
			syslog(LOG_ERR,"Old profile %s not exist. Failed to configure.",
				cfg->cprof());
			PDEBUG(DERR, "Old profile %s not exist. Failed to configure.",
				cfg->cprof());
			return -1;
		}
	}

	// If this interface configured manually or
	// its configuration not changed - return
	if (!cfg->can_apply())
		return 0;
	// Configure device
	PDEBUG(DERR, "Configure dev");
	// apply Local settings (if there is some)
	int lchng = 0, pchng = 0;
	local_configure(lchng);
	if (ret = dev->configure(prof->conf, (type == slave) ? 0 : 1, pchng)) {
		PDEBUG(DERR, "Error while configure device %s", ch_name);
		syslog(LOG_ERR,"Error while configure device %s", ch_name);
	}
	PDEBUG(DERR, "Local_ch=%d, Prof_ch=%d", lchng, pchng);
	// If ret = 0 => interface setted without errors
	// lchng || pchng - means that apply is needed
	if (!ret && (lchng || pchng)) { // some onfiguration is hanged
		PDEBUG(DERR, "commit");
		dev->commit();
	}
	return ret;
}

