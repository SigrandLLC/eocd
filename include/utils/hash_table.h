#ifndef SIGRAND_HASH_TABLE_H
#define SIGRAND_HASH_TABLE_H

#define HASH_SIZE 200
#include <generic/EOC_types.h>
#include <eoc_debug.h>
#include <string.h>
#include <list>
using namespace std;

#define EOC_DEBUG
#include <generic/EOC_generic.h>

class hash_elem{
  public:
    char *name;
    int nsize;
    hash_elem *next;
    hash_elem *prev;
    u8 is_updated;

    inline bool operator < (hash_elem &right){
      int len = right.nsize > nsize ? nsize : right.nsize;
      if( strncmp(name,right.name,len) < 0 )
        return 1;
      return 0;
    }

    ~hash_elem(){
    	PDEBUG(DFULL,"destruct %s hash element",name);
    	free(name);
    	PDEBUG(DFULL,"end");
    }
};


class hash_table{
  private:
    list< hash_elem *> table[HASH_SIZE];
    int _hash(const char *name);
    int max_hash_name;
    // sequential list of table items
    hash_elem *head,*tail;
  public:
    hash_table(int mhash_name);
    ~hash_table();
    hash_elem *find(const char *name, int nsize);
    int add(hash_elem *el);
    hash_elem *del(const char *name,int nsize);
    //hash_elem *del_nofree(char *name,int nsize);
    void sort();
    // sequential trace
    inline hash_elem *first(){ return head; }
    inline hash_elem *next(char *name, int nsize){
      hash_elem *el = find(name,nsize);
      if( !el )
        return NULL;
      return el->next;
    }
    // Clear table from non actual rows (which wasnt updated)
    void clear(void (*del_func)(hash_elem *el));
};

#endif
