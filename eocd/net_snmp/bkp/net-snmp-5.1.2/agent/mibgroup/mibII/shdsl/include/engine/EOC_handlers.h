/*
 * EOC_responses.h:
 *	Contains structures of SHDSL EOC responses
 */

#ifndef EOC_HANDSLERSS_H
#define EOC_HANDLERS_H

#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <db/EOC_db.h>
#include <config/EOC_config.h>
#include <engine/EOC_scheduler.h>
#include <engine/EOC_responder.h>


typedef EOC_scheduler::sched_state sched_state;

// Poller Request handler prototype
typedef EOC_msg *(*request_handler_t)(sched_state stat,sched_elem el,EOC_config *cfg);
 
#endif
