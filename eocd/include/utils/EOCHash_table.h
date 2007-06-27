#ifndef SIGRAND_HASH_TABLE_H
#define SIGRAND_HASH_TABLE_H

#define HASH_SIZE 200

#include <string.h>
#include "EOClist.h"

class EOCHash_data{

};

class EOCHash_el: public EOCList_elem{
private:
    char *name;
    int nsize;
    EOCHash_data *data;
public:
    EOCHash_el(){
	nsize = 0;
	name = NULL;
	data = NULL;
    }
    
    ~EOCHash_el(){
	if( name )
	    delete[] name;
	if(data)
	    delete data;
    }

    inline void setname(char *n,int sz){
	name = new char[sz];
	strncpy(name,n,sz);
	nsize = sz;
    }
    inline void setdata(EOCHash_data *d){
	data = d;
    }

    inline EOCHash_data *getdata(){
	return data;
    }
    
    inline int check_ident(char *n){
	if( strncmp(name,n,nsize) == 0 )
	    return 1;
	return 0;
    }
};
    

class EOCHash_table
{
private:
    EOClist *table[HASH_SIZE];
    int _hash(char *name);
public:
    EOCHash_table();
    ~EOCHash_table();
    EOCHash_data *find(char *name);
    int add(char *name,int nsize,EOCHash_data *data);
    int del(char *name);
};

#endif
