#ifndef EOC_DEBUG_H
#define EOC_DEBUG_H


#define EOC_DEBUG

#ifdef EOC_DEBUG
#	include <cassert>
#	define ASSERT(x) assert(x)
#else
#	define ASSERT(x) 
#endif

#endif
