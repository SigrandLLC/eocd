#ifndef SIGRAND_EOC_GENERIC_H
#define SIGRAND_EOC_GENERIC_H

#include <generic/EOC_types.h>

typedef enum { master,slave,repeater } dev_type;
typedef enum { eoc_Offline, eoc_Discovery, eoc_Online } shdsl_state;
typedef enum {err =-1,unknown=0 ,stu_c,stu_r,sru1,sru2,sru3,sru4,sru5,sru6,
			sru7,sru8,sru9,sru10,BCAST=0xf} unit;
typedef enum { no_side=-1,net_side=0,cust_side } side;

// SHDSL standard related
#define MAX_LOOPS 4
#define MAX_UNITS 10
#define MAX_REPEATERS (MAX_UNITS-2)
#define PBO_SETTING_LEN 3*16

// EOC messages
#define REQUEST_QUAN 128
#define RESPONSE_QUAN 128
#define RESP_OFFSET 128



// OS related
#define FNAME_SIZE 32
#define PATH_SIZE 256
#define FILE_PATH_SIZE PATH_SIZE+FNAME_SIZE
#define IF_NAME_LEN 32
#define PCI_NAME_LEN 32
#define PCI_MAX_SLOTS 8
#define MOD_MAX_DEVS 8
#define DEVICE_PATH "/sys/bus/pci/devices"
#define OS_IF_PATH "/sys/class/net"

#endif

