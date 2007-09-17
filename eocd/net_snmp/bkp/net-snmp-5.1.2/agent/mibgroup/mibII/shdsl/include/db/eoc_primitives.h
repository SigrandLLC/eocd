#ifndef SIGRAND_EOC_PRIMS_H
#define SIGRAND_EOC_PRIMS_H

// Define types
typedef unsigned int u32;
typedef int s32;
typedef unsigned short u16;
typedef short s16;
typedef unsigned char u8;
typedef char s8;

//-----------------------------------------------------------------------

#define ADMIN_STR_LEN 32

typedef enum { region1=0,region2=1} transm_mode_t;
typedef enum { xtuC=1,xtuR,xru1,xru2,xru3,xru4,xru5,xru6,xru7,xru8 } unit_id_t;
typedef enum { pair1=1,pair2,pair3,pair4 } pair_num_t;
typedef enum { network=1,customer } side_t;
typedef enum { normal=1,resrved} tip_ring_rev_t;
typedef enum { preActivation=1, activation, data} act_state_t;
typedef enum { noLoopback = 1, normalLoopback, specialLoopback } loopback_cfg_t;
typedef enum { def = 1,ench } power_backoff_t;
typedef enum { ready = 1,restart } soft_restart_t;
typedef enum { local = 1, span } pwr_src_t;
typedef enum { twoWire = 1,fourWire,sixWire,eightWire } wire_if_t;
typedef enum { symmetric = 1, asymmetric } PSD_t;
typedef enum { localClk = 1, networkClk, dataOrNetworkClk, dataClk } clock_ref_t;
typedef enum { noPower = 1, powerFeed, wettingCurrent } remote_conf_t;
typedef enum { disabled = 1, enabled } pwr_feeding_t;
typedef enum { disable = 1, enable } line_probe_t;
typedef enum { active = 1, notInService, notReady, createAndGo, createAndWait, destroy } row_statuis_t;


typedef struct {
	u16 noDefect :1;
	u16 powerBackoff :1;
	u16 deviceFault :1;
	u16 dcContinuityFault :1;
	u16 snrMarginAlarm :1;
	u16 loopAttenuationAlarm :1;
	u16 loswFailureAlarm :1;
	u16 configInitFailure :1;
	u16 protocolInitFailure:1;
	u16 noNeighborPresent :1;
	u16 loopbackActive :1;
} status_bits_t;

typedef struct {
	u8 currCondDown :1;
	u8 worstCaseDown :1;
	u8 currCondUp :1;
	u8 worstCaseUp :1;	
} targ_marg_t;

// Perfomance & status characteristics
typedef struct{
    u32 es;
    u32 ses;
    u32 crc_anom;
    u32 losws;
    u32 uas;
} perf_stat_t;
		    


#endif
