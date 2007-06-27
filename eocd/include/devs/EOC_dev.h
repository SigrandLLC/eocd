#ifndef SIGRAND_EOC_DEV_H
#define SIGRAND_EOC_DEV_H

#define EOC_DEV_NAMESZ 256
#include <generic/EOC_generic.h>
#include <generic/EOC_msg.h>


class EOC_dev{
public:
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
    
protected:
    int error_init;
    device dev_type;
    EOC_dev_cfg *cfg;
public:
    virtual int send(EOC_msg *msg) = 0;
    virtual EOC_msg *recv() = 0;
/*    virtual read_cfg(EOC_dev_cfg *cfg) = 0;
    virtual apply_cfg(EOC_dev_cfg *cfg) = 0;    
*/
};

#endif
