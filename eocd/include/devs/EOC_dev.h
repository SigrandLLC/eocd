#ifndef SIGRAND_EOC_DEV_H
#define SIGRAND_EOC_DEV_H

#define EOC_DEV_NAMESZ 256
#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_responses.h>


class EOC_dev{
public:
    typedef enum{ OFFLINE, ONLINE } Linkstate  ;
    enum device { SG16PCI,SG17PCI,SG17R };
    enum annex_t { annexA,annexB,annexF };
    enum tcpam_t { tcpam4, tcpam8, tcpam16,tcpam32 };
    enum terminator_t { master,slave };
    typedef struct {
	unsigned int lrate;
	annex_t annex;
	tcpam_t tcpam;
	terminator_t term;
    } EOC_dev_cfg;
    typedef struct {
	s8 snr_marg, loop_attn;
	u8 es,ses,losws,uas;
	u16 crc, sega;
	u8 seg_def,cntr_ovfl,cntr_rst;
    } shdsl_statistics;
    typedef struct {
	u8 master:1;
	u8 tcpam :3;
	u8 annex :3;
	u32 rate;
    } shdsl_settings;
protected:
    int error_init;
    device dev_type;
    int valid;
public:
    // Initialisation check
    int init_ok() { return valid; }
    //----- EOC functions ---------//
    virtual int send(EOC_msg *msg) = 0;
    virtual EOC_msg *recv() = 0;
    //----- SHDSL channel ----------//
    virtual int loops() = 0;
    virtual Linkstate link_state() = 0;
    //----- SHDSL settings ---------//
    virtual int tresholds(s8 loop_attn,s8 snr_m) = 0;
    virtual int statistics(int loop,side_perf &stat) = 0;


};


#endif
