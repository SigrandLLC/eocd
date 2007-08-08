#include <app_frame.h>

int main()
{
    app_frame a(app_frame::SPAN_CONF,app_frame::GET,app_frame::REQUEST,"dsl0");
    app_frame b(app_frame::SPAN_CONF,app_frame::GET,app_frame::REQUEST);
    printf("chan name: %s",a.chan_name());






}