#include <stdio.h>
#include <time.h>

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;


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
	EOC_ring_container(int l)
	{
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


char* printtime(time_t tm){
	static char str[256];
	strftime(str, 256, "%d.%m.%y.%R", localtime(&tm));
	return str;
}

class sens_events {
private:
	time_t _start;
	time_t _end;
	int _cnt;
public:
	sens_events(){
		_start = _end = 0;
		_cnt = 0;
	}
	sens_events(time_t start){
		_start = start; 
		_end = 0;
		_cnt = 1;
	}

	
	int event_start(time_t start)
	{
		printf("start event: %s\n",printtime(start));
		_start = start;
		_cnt++;
	}
	bool event_started(){
		return (_start) > 0 ? true : false;
	}

	void event_end(time_t end){
		printf("end event: %s\n",printtime(end));
		_end = end;
	}
	bool event_colsed(){
		return (_end) > 0 ? true : false;
	}	
	bool related_event(time_t start){
		if( _end ){
			if( (start-_end) < 2*60 && (_start - _end)< 15*60 &&
				_cnt<100 )
				return true;
			else
				return false;
		}
		return true;
	}
	
	void event_add(time_t start){
		printf("add event: %s\n",printtime(start));
		if( _end ){
			_end = 0;
		}
		_cnt++;
	}
	int event_descr(time_t &start,time_t &end){
		start = _start;
		end = _end;
		return _cnt;
	}
};

int main()
{
	EOC_ring_container<sens_events> sens(100);
	time_t times[][2] = { {1,0}, {1+1*60,1+1.5*60}, {1 + 2*60,1 + 2.5*60}, {1 + 10*60,1 + 10.5*60}, {1+40*60,1+40.5*60}, 
			{1 + 41*60,1 + 41.5*60}, {1+44*60,1*50*60} };

	int times_size = sizeof(times)/(sizeof(time_t)*2);

	
	// 1st event
	for( int i=0;i<times_size; i++){
		if( !sens[0]->event_started() ){
			sens[0]->event_start(times[i][0]);
			if( times[i][1] ){
				sens[0]->event_end(times[i][1]);
			}
		}else{
			if( sens[0]->related_event(times[i][0]) ){
				sens[0]->event_add(times[i][0]);
			}else{
				sens.shift(1);
				sens[0]->event_start(times[i][0]);
			}
			if( times[i][1] )
				sens[0]->event_end(times[i][1]);
		}
	}

	printf("Events description:\n");
	for(int i=0;i<100 && sens[i]; i++){
		time_t start,end;
		int count;
		count= sens[i]->event_descr(start,end);
		printf("%s -",printtime(start));
		printf("%s : ",printtime(end));
		printf("%d\n",count);
	}
}
