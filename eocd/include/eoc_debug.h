#ifndef EOC_DEBUG_H
#define EOC_DEBUG_H

#include <stdio.h>
#include <syslog.h>
//#define EOC_DEBUG

extern int debug_lev;
extern int daemon_mode;

#define DOFF -1
#define DERR 0
#define DINFO 5
#define DFULL 10
#define DNDISP 15

#ifdef EOC_DEBUG
#	include <cassert>
#	define ASSERT(x) assert(x)
#   define PDEBUG(lev,fmt,args...)							\
{ \
	if( lev<=debug_lev && !daemon_mode ){ \
		printf("%s(%s) : " fmt " \n",__FILE__,__FUNCTION__, ## args  ); \
		fflush(stdout); \
	} else if( lev<=debug_lev && daemon_mode ){ \
		syslog(LOG_NOTICE,"%s(%s) : " fmt " \n",__FILE__,__FUNCTION__, ## args  ); \
	} \
}

#   define PDEBUGL(lev,fmt,args...)							\
	if( lev<=debug_lev )										\
		printf(fmt,## args)


#	define EDEBUG(lev,function)					\
	if( lev<=debug_lev && !daemon_mode )						\
		function

#	define PERROR(fmt,args...) \
	if( daemon_mode ){ \
			syslog(LOG_NOTICE,"%s " fmt " : %s\n", __FUNCTION__, ##args,strerror(errno)); \
	}else{ \
		printf("eocd(%s) " fmt " : %s\n", __FUNCTION__, ##args,strerror(errno)); \
	}

#else

#	define ASSERT(x)
#	define PDEBUG(lev,fmt,args...)
#	define EDEBUG(lev,function)
#	define PERROR(fmt,args...)

#endif


#endif


