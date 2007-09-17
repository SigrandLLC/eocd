#ifndef EOC_TYPES_H
#define EOC_TYPES_H

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;

typedef struct{
    u16 noDefect:1;
    u16 powerBackoff:1;
    u16 deviceFault:1;
    u16 dcContFault:1;
    u16 snrMargAlarm:1;
    u16 loopAttnAlarm:1;
    u16 loswFailAlarm:1;
    u16 configInitFailure:1;
    u16 protoInitFailure:1;
    u16 noNeighborPresent:1;
    u16 loopbackActive:1;
} shdsl_status_t;
 

typedef struct{
    u32 es;
    u32 ses;
    u32 crc;
    u32 losws;
    u32 uas;
} counters_t;


#endif
