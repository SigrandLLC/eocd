#ifndef SIGRAND_LIST_H
#define SIGRAND_LIST_H

class EOCList_elem{

};

class __list{
private:
    EOCList_elem *data;
    __list *next;
public:
    __list(){
	data = NULL;
	next = NULL;
    }
    ~__list(){
	if( data )
	    delete data;
    }
    inline EOCList_elem *get_data(){
	return data;
    }
    inline __list *get_next(){
	return next;
    }
    inline void set_data(EOCList_elem *d){
	data = d;
    }
    inline void set_next(__list *n){
	next = n;
    }
};

class EOClist{
private:
    __list *head;
    __list *tail;
    __list *current;
    int size;

    __list *prev_by_ptr(__list *);
public:
    EOClist();
    ~EOClist();
    int head_current();
    int next();
    int add_first(EOCList_elem *d);
    int add_last(EOCList_elem *d);
    EOCList_elem *get_cur();
    EOCList_elem *get_first();
    EOCList_elem *get_tail();
    int del_cur();
    int del_first();
    int del_last();
    // iteration through list
};


// TODO: make as objects rather than include!!!

#endif
