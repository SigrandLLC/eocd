#ifndef SIGRAND_LOG_H
#define SIGRAND_LOG_H

#define LOG_ERR 0
#define LOG_WARN 1


class EOCLog_config{



};


class EOCLog{
        
public:
    void print(int,char*);
}


void
EOCLog::print(int facility,char* s)
{
    char sfacility[256];
    
    switch( facility ){
    case LOG_ERR:
	sfacility = strdup("Error");
	break;
    case LOG_WARN:
	sfacility = strdup("Warning");
	break;
    default:
	sfacility = strdup("Default");
    }
    
    printf("%s %s\n",sfacility,s);
}


#endif