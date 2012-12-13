#include <stdio.h>
#include "EOClist.h"

__list *
EOClist::prev_by_ptr(__list *s)
{
    __list *el = head;
    if( !el )
	return NULL;
    if( head == s )
	return el;

    while( el->get_next() != s && el->get_next()!=NULL )
	el = el->get_next();

    if( el->get_next() == s )
	return el;
    return NULL;
}

EOClist::EOClist()
{
    head = tail = current = NULL;
    size = 0;
}

EOClist::~EOClist()
{
    while( !del_first() );
}

int
EOClist::head_current()
{
    if( !head )
	return -1;
    current = head;
    return 0;
}

int
EOClist::next()
{
    if( !current || !head || !current->get_next() )
	return -1;
    current = current->get_next();
    return 0;
}

int
EOClist::add_first(EOCList_elem *d)
{
    __list *el = new __list;
    el->set_data(d);
    el->set_next(head);
    if( !head )
	tail = current = el;
    head = el;
    size++;
    return 0;
}


int
EOClist::add_last(EOCList_elem *d)
{
    if( head && !tail )
	return -1;
    if( !head )
	return add_first(d);

    __list *el = new __list;
    el->set_data(d);
    el->set_next(NULL);
    tail->set_next(el);
    tail = el;
    size++;
    return 0;
}

EOCList_elem*
EOClist::get_cur()
{
    if( !head || !current )
	return NULL;
    return current->get_data();
}

EOCList_elem*
EOClist::get_first()
{
    if( !head )
	return NULL;
    return head->get_data();
}

EOCList_elem*
EOClist::get_tail()
{
    if( !tail )
	return NULL;
    return tail->get_data();
}


int
EOClist::del_first()
{
    __list *el = head;

    if( !head )
	return -1;

    if( head == tail ){
	size = 0;
	delete head;
	head = tail = current = NULL;
	return 0;
    }

    head = head->get_next();
    if( current == el )
	current = head;
    delete el;
    size--;

    return 0;
}

int
EOClist::del_last()
{
    __list *el = head;
    if( head == tail )
	del_first();
    if( (el = prev_by_ptr(tail)) !=NULL ){
	if( current == tail )
	    current = head;
	delete tail;
	el->set_next(NULL);
	tail = el;
	size--;
	return 0;
    }
    return -1;
}


int
EOClist::del_cur()
{
    __list *el;

    if( !current )
	return -1;

    if( current == head )
	return del_first();

    if( current == tail )
	return del_last();

    if( (el = prev_by_ptr(current)) !=NULL ){
	el->set_next(current->get_next());
	delete current;
	current = head;
	size--;
	return 0;
    }
    return -1;
}

