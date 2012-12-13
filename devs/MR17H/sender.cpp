#include <devs/EOC_mr17h.h>
#include <generic/EOC_msg.h>
#include <generic/EOC_requests.h>

int main()
{
    EOC_mr17h dev("dsl1");
    EOC_msg m(20);
    m.src(stu_c);
    m.dst(stu_r);
    m.type(REQ_DISCOVERY);
    sprintf(m.mptr(),"Sigrand-try");
    dev.send(&m);
}