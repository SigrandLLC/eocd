#include <stdio.h>

#include <span_profile.h>
#include <devs/EOC_mr17h.h>

int main()
{
    EOC_mr17h dev("dsl0");
    span_conf_profile_t prof;
    memset(&prof,0,sizeof(prof));
    prof.annex = annex_b;
    prof.max_rate = 5696;
    dev.configure();
}