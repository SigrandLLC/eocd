#include <stdio.h>
#include <string.h>
#include "EOCHash_table.h"

int
EOCHash_table::_hash(char *name)
{
    int i=0, sum=0;
    
    while( name[i]!='\0' ){
	sum+=name[i];
	i++;
    }
    return (sum % HASH_SIZE);
}
				

EOCHash_table::EOCHash_table()
{
    int i;
    for(i=0;i<HASH_SIZE;i++)
	table[i] = new EOClist;
}

EOCHash_table::~EOCHash_table()
{
    int i;
    for(i=0;i<HASH_SIZE;i++){
	if( table[i] ){
	    while( !table[i]->del_first() );
	}
    }
}

EOCHash_data*
EOCHash_table::find(char *name)
{
    EOCHash_el *el;
    int i = _hash(name);

    if( table[i]->head_current() )
	return NULL;
    do{
	el = (EOCHash_el*)table[i]->get_cur();
	if( el->check_ident(name) )
	    return el->getdata();
    }while( !table[i]->next() );
    return NULL;
}

int
EOCHash_table::add(char *name,int nsize,EOCHash_data *data)
{
    EOCHash_el *el;
    int i = _hash(name);    

    if( find(name) != NULL )
	return -1;

    el = new EOCHash_el;
    el->setname(name,nsize);
    el->setdata(data);

    if(table[i]->add_last(el)) 
	return -1;
    return 0;
}

int
EOCHash_table::del(char *name)
{
    EOCHash_el *el;
    int i = _hash(name);

    if( table[i]->head_current() )
	return -1;
    do{
	el = (EOCHash_el*)table[i]->get_cur();
	if( el->check_ident(name) ){
	    table[i]->del_cur();
	    return 0;
	}
    }while( !table[i]->next() );
    return -1;
}

