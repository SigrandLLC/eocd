#ifndef EOC_RESP_HANDLERS_H
#define EOC_RESP_HANDLERS_H

#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>
#include <generic/EOC_responses.h>
#include <engine/EOC_handlers.h>
#include <engine/EOC_router.h>

int _inventory(EOC_router *dev,EOC_msg *m);
int _configure(EOC_router *dev,EOC_msg *m);
int _test(EOC_router *dev,EOC_msg *m);

#endif