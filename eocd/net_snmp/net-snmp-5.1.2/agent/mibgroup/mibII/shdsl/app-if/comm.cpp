#include <app-if/app_frame.h>
#include <app-if/app_comm_cli.h>


extern "C" {

app_comm_cli *
init_comm()
{
    app_comm_cli *cli = new app_comm_cli("/var/eocd/eocd-socket");
    if( !cli->init_ok() ){
	delete cli;
	return NULL;
    }
    return cli;
}

char *
comm_alloc_request(app_ids id,app_types type,char *chname,app_frame **fr)
{
    *fr = new app_frame(id,type,app_frame::REQUEST,1,chname);
    if( !(*fr)->frame_ptr() ){
	printf("Error while creating frame!\n");
	return NULL;
    }
    printf("Ok creating frame, ptr=%p\n",(*fr)->frame_ptr());
    
    return (*fr)->payload_ptr();
}

app_frame *
comm_request(app_comm_cli *comm,app_frame *fr)
{
    char *b;
    comm->send(fr->frame_ptr(),fr->frame_size());
    comm->wait();
    int size = comm->recv(b);
    app_frame *fr1 = new app_frame(b,size);
    if( !fr1->frame_ptr() || fr1->is_negative() ){
	delete fr1;
	return NULL;
    }
    return fr1;
}

char *
comm_frame_payload(app_frame *fr)
{
    if( !fr->frame_ptr() )
	return NULL;
    return fr->payload_ptr();
}

void 
set_chan_name(app_frame *fr,char *name)
{
    fr->chan_name(name);
}

void comm_frame_free(app_frame *fr){
    delete fr;
}

void comm_free(app_comm_cli *cli){
    delete cli;
}

}

