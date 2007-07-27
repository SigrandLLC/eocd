#ifndef SIGRAND_HASH_TABLE_H
#define SIGRAND_HASH_TABLE_H

#define HASH_SIZE 200
#include <string.h>
#include <list>
using namespace std;


class hash_elem{
public:
    char *name;
    int nsize;
    inline bool operator < (hash_elem &right){
        int len = right.nsize > nsize ? nsize : right.nsize;
        if( strncmp(name,right.name,len) < 0 )
    	    return 1;
        return 0;
    }
};


class hash_table{
private:
    list< hash_elem *> table[HASH_SIZE];
    int _hash(char *name);
    int max_hash_name;
    // trace variables
    int t_iter;
    list< hash_elem *>::iterator l_iter;
public:
    hash_table(int mhash_name);
    ~hash_table();
    hash_elem *find(char *name, int nsize);
    int add(hash_elem *el);
    int del(char *name,int nsize);
    // sequential trace
    int init_trace();
    hash_elem *next_elem();
    
};

#endif