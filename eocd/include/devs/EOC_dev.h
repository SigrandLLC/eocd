#ifndef SIGRAND_EOC_DEV_H
#define SIGRAND_EOC_DEV_H

#define EOC_DEV_NAMESZ 256
#include <generic/EOC_types.h>
#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>


class EOC_dev{
public:
    enum Linkstate { OFFLINE, ONLINE };
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
    EOC_dev_cfg *cfg;
public:
    //----- EOC functions ---------//
    virtual int send(EOC_msg *msg) = 0;
    virtual EOC_msg *recv() = 0;
    //----- SHDSL channel ----------//
    virtual Linkstate link_state() = 0;
//    virtual int statistics(shdsl_statistics *s) = 0;
    //----- SHDSL settings ---------//
    virtual int tresholds(s8 loop_attn,s8 snr_m) = 0;
/*    virtual int config( shdsl_config *cfg ) = 0;
    virtual shdsl_config *config() = 0;
*/

    // loop status
    virtual u8 loops() = 0;
    virtual int status_collect() = 0;
    virtual int perf_change(u8 loop) = 0;
    virtual u8 losws_alarm(u8 loop) = 0;
    virtual u8 loop_attn_alarm(u8 loop) = 0;
    virtual u8 snr_marg_alarm(u8 loop) = 0;
    virtual u8 dc_cont_flt(u8 loop) = 0;
    virtual u8 dev_flt(u8 loop) = 0;
    virtual u8 pwr_bckoff_st(u8 loop) = 0;
    virtual s8 snr_marg(u8 loop) = 0;
    virtual s8 loop_attn(u8 loop) = 0;
    virtual u8 es(u8 loop) = 0;
    virtual u8 ses(u8 loop) = 0;
    virtual u8 crc(u8 loop) = 0;
    virtual u8 losws(u8 loop) = 0;
    virtual u8 uas(u8 loop) = 0;
    virtual u8 pwr_bckoff_base_val(u8 loop) = 0;
    virtual u8 cntr_rst_scur(u8 loop) = 0;
    virtual u8 cntr_ovfl_stur(u8 loop) = 0;
    virtual u8 cntr_rst_scuc(u8 loop) = 0;
    virtual u8 cntr_ovfl_stuc(u8 loop) = 0;
    virtual u8 pwr_bkf_ext(u8 loop) = 0;
};




/*
 
*/
#endif
