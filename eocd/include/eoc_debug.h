#ifndef EOC_DEBUG_H
#define EOC_DEBUG_H

#include <cassert>

#ifdef EOC_DEBUG
#	define ASSERT(x) assert(x)
#else
#	define ASSERT(x) 
#endif

#endif
