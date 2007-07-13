#ifndef SIGRAND_EOC_GENERIC_H
#define SIGRAND_EOC_GENERIC_H

#define EOC_MAX_UNITS 10
#include <generic/EOC_types.h>

enum dev_type { master,slave,repeater };
enum shdsl_state { eoc_Offline, eoc_Discovery, eoc_Online };
enum unit {err =-1,unknown=0 ,stu_c,stu_r,sru1,sru2,sru3,sru4,sru5,sru6,
			sru7,sru8,sru9,sru10,BCAST=0xf};

// EOC messages
#define REQUEST_QUAN 128
#define RESPONSE_QUAN 128
#define RESP_OFFSET 128

#endif

