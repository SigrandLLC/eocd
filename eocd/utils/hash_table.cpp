#include <stdio.h>
#include <string.h>
#include <eoc_debug.h>
#include <utils/hash_table.h>

hash_table::hash_table(int mhash_name){
    max_hash_name = mhash_name;
    for(int i=0;i<HASH_SIZE;i++)
		table[i].clear();
    // clear sequential list
    head = NULL;
    tail = NULL;
}

hash_table::~hash_table(){
    for(int i=0;i<HASH_SIZE;i++){
		while( table[i].size() ){
			hash_elem *ptr = *table[i].begin();
			table[i].pop_front();
			free(ptr->name);
			delete ptr;
		}
    }
}

int
hash_table::_hash(char *name)
{
    int i=0, sum=0;
    
    while( name[i]!='\0' ){
		sum+=name[i];
		i++;
    }
    return (sum % HASH_SIZE);
}


			
hash_elem *
hash_table::find(char *name,int nsize)
{
    int i = _hash(name);

    if( !table[i].size() ){ // hash list is empty
		return NULL;
	}

    list<hash_elem *>::iterator p = table[i].begin();

    for(;p!=table[i].end();p++){
		hash_elem *ptr = *p;
		int len = (ptr->nsize > nsize) ? nsize : ptr->nsize;
		if( !strncmp(ptr->name,name,len) ){
			return ptr;
		}
    }
    return NULL;
}

int
hash_table::add(hash_elem *el)
{
    ASSERT( el );
    int len = ( el->nsize < max_hash_name ) ? el->nsize : max_hash_name;
    int i = _hash(el->name);    

    el->next = el->prev = NULL;
    hash_elem *exist = find(el->name,el->nsize);
    if( exist )
		return -1;
    // add to hash table
    table[i].push_back(el);
    
    // add to sequential list
    if( !head ){
		head = el;
		tail = el;
    }else{
		tail->next = el;
		el->prev = tail;
		tail = el;
    }

    return 0;
}

hash_elem *
hash_table::del(char *name,int nsize)
{

    int i = _hash(name);
    if( !table[i].size() ) // hash list is empty
		return NULL;
    list<hash_elem *>::iterator p;
    for(p = table[i].begin();p!=table[i].end();p++){
		hash_elem *ptr = *p;
		int len = ( ptr->nsize > nsize) ? nsize : ptr->nsize;
		if( !strncmp( ptr->name,name,len) ){
			// delete from hash-table
			table[i].erase(p);
			// delete from sequential list
			if( ptr->prev && ptr->next ){
				ptr->prev->next = ptr->next;
				ptr->next->prev = ptr->prev;
			}else{
				if( !ptr->prev){
					ASSERT( head == ptr );
					head = ptr->next;
					if( head )
						head->prev = NULL;
				}
				if( !ptr->next ){
					ASSERT( tail == ptr );
					tail = ptr->prev;
					if( tail )
						tail->next = NULL;
				}
			}
			hash_elem *nx = ptr->next;
			delete ptr;
			return nx;
		}
    }
    return NULL;
}

void
hash_table::clear()
{
    hash_elem *el = first();
    while( el ){
		PDEBUG(DERR,"Process %s profile",el->name);
		if( !el->is_updated ){
			PDEBUG(DERR,"Delete row: %s",el->name);
			el = del(el->name,el->nsize);
			PDEBUG(DERR,"Next is: %s",el->name);
		}else{
			PDEBUG(DERR,"Keep profile %s",el->name);
			el->is_updated = 0;
			el = next(el->name,el->nsize);
		}
    }
	// DEBUG PRINT
    el = first();
	PDEBUG(DERR,"Print new conf table");
    while( el ){
		PDEBUG(DERR,"\t%s",el->name);
		el = next(el->name,el->nsize);
    }
	
}

// BUBLE sort. TODO: use more optimal
void 
hash_table::sort(){
	int flag = 1;

	while( flag ){
		flag = 0;
		hash_elem *el = first();
		while( el && el->next ){
			hash_elem *nel = el->next;
			int len = (el->nsize > nel->nsize) ? nel->nsize : el->nsize;
			if( strncmp(el->name,nel->name,len) > 0 ){
				flag = 1;
				// Change sequence in list
				if( el == head && nel == tail ){ // Have only 2 elements in list
					tail = el;
					head = nel;
					el->prev = nel;
					el->next = NULL;
					nel->next = el;
					nel->prev = NULL;
				}else if( el == head ){
					head = nel;
					el->next = nel->next;
					el->next->prev = el;
					el->prev = nel;
					nel->next = el;
					nel->prev = NULL;
				}else if( nel == tail ){
					tail = el;
					nel->next = el;
					nel->prev = el->prev;
					el->prev->next = nel;
					el->prev = nel;
					el->next = NULL;
				}else{
					el->prev->next = nel;
					nel->next->prev = el;
					el->next = nel->next;
					nel->prev = el->prev;
					el->prev = nel;
					nel->next = el;
				}
				
			}else{
				el = nel;
			}
		}
	}
}
