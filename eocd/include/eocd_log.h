#ifndef SIGRAND_EOCD_LOG_H
#define SIGRAND_EOCD_LOG_H

enum levels { WARNING, ERROR };

#define CONFL 0

#define eocd_perror(fmt,args...) printf("eocd(%s) " fmt " : %s\n", __FUNCTION__, ##args,strerror(errno))
#define eocd_log(lev,fmt,args...) printf("eocd(%s) " fmt "\n", __FUNCTION__, ##args)

#endif
