#ifndef SHDSL_COMM_H
#define SHDSL_COMM_H

#include <generic/EOC_types.h>
#include <app-if/app_messages.h>

struct app_comm;
struct app_frame;

struct app_comm *init_comm(int q);
char *comm_alloc_request(app_ids id,app_types type,char *chname,struct app_frame **fr);
struct app_frame *comm_request(struct app_comm *comm,struct app_frame *fr);
char *comm_frame_payload(struct app_frame *fr);
void set_chan_name(struct app_frame *fr,char *name);
void comm_frame_free(struct app_frame *fr);
void comm_free(struct app_comm *cli);

#endif
