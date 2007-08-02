#ifndef EOC_DEBUG_H
#define EOC_DEBUG_H


#define EOC_DEBUG

#ifndef DEFAULT_LEV 
#	define DEFAULT_LEV 0
#endif


#ifdef EOC_DEBUG
#	include <cassert>
#	define ASSERT(x) assert(x)
#       define PDEBUG(lev,fmt,args...) \
		if( lev<=DEFAULT_LEV ) \
			printf("eocd: %s " fmt " \n",__FUNCTION__, ## args  )
#else
#	define ASSERT(x) 
#	define PDEBUG(lev,fmt,args...)
#endif


#endif


