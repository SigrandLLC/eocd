#ifndef SIGRAND_EOC_ENGINE_H
#define SIGRAND_EOC_ENGINE_H


#define MAX_HANDLERS 256

class EOCEngine{
    EOCHandler *table[MAX_HANDLERS];
    EOCDB *db;
public:
    EOCEngine();
    ~EOCEngine();    
    int register_handler(int num,EOCHandler *h);
    int unregister_handler(int num);
    process_msg(int chan char *msg);
};

EOCEngine::EOCEngine()
{
    int i;
    for(i=0;i<MAX_HANDLERS;i++)
	table[i] = NULL;
    db = NULL;
}

EOCEngine::~EOCEngine(){ }


int EOCEngine::
register_handler(unsigned int num,EOCHandler *h)
{
    if( num >= MAX_HANDLERS || table[num] ){
	return -1;
    }
    table[num] = h;	
    return 0;
}


int EOCEngine::
unregister_handler(unsigned int num)
{
    if( num >= MAX_HANDLERS || !table[num] ){
	return -1;
    }
    table[num] = NULL;
    return 0;
}


int EOCEngine::
process_msg(int chan, char *msg)
{
    
}


#endif