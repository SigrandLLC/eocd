#ifndef EOC_RING_CONTAINER_H
#define EOC_RING_CONTAINER_H

#include <generic/EOC_types.h>

template<class T>
class EOC_ring_container {
private:
	T **mas;
	u32 len;
	u32 head, qlen;

	inline void inc_head()
	{
		head = (head+1)%len;
	}
	inline int ind(int i)
	{
		if(!(i<len))
			return -1;
		if(head<i)
			return (head+len-i);
		return (head-i);
	}

public:

	EOC_ring_container(int l = 1)
	{
		mas = new T*[l];
		for(int i = 0;i<l;i++){
			mas[i] = NULL;
		}
		len = l;
		head = 0;
		mas[head] = new T;
	}

	void resize(int l)
	{
		// free previous array
		for(int i=0;i<len;i++){
			if( mas[i] )
				delete mas[i];
		}
		delete[] mas;
		// Create new one
		mas = new T*[l];
		for(int i = 0;i<l;i++){
			mas[i] = NULL;
		}
		len = l;
		head = 0;
		mas[head] = new T;
	}
	
	
	~EOC_ring_container<T> ()
	{
		for(int i = 0;i<len;i++)
			if(mas[i])
				delete mas[i];
		delete[] mas;
	}

	T * operator[](unsigned int i)
	{
		int ret = ind(i);
		if(ret<0)
			return NULL;
		return mas[ret];
	}

	void shift(u32 ints)
	{
		u32 nhead = (head+ints)%len;
		u32 nrings = ints/len;

		if(!ints)
			return;

		if(nrings){
			for(int i = 0;i<len;i++){
				if(mas[i]){
					delete mas[i];
					mas[i] = NULL;
				}
			}
			head = nhead;
		}else{
			while(head!=nhead){
				inc_head();
				if(mas[head]){
					delete mas[head];
					mas[head] = NULL;
				}
			}
		}
		mas[head] = new T;
	}
};

#endif
