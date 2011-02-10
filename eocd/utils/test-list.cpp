#include <stdio.h>
#include <stdlib.h>
#include "EOClist.cpp"


class list_elem : public EOCList_elem{
public:
    int val;
    list_elem(int v){
        val = v;
    }
};

int main()
{
    EOClist *l = new EOClist;
    int i;
    list_elem *a;

int k;
for(k=0;k<10;k++){

    for(i=0;i<12;i++){
	a = new list_elem(i);
	l->add_first(a);
    }
    printf("print:\n");

    if( !l->head_current() ){
        do{
		printf("%d ",((list_elem*)l->get_cur())->val);
        }while( !l->next() );
    }

    printf("\n");

    printf("delete:\n");
    while(!l->del_last());

    printf("print:\n");

    if( !l->head_current() ){
        do{
		printf("%d ",((list_elem*)l->get_cur())->val);
        }while( !l->next() );
    }

    printf("\n");

}
    return 0;
}






/*
    hash_table_t table, *t = &table;
    int i, a[]={1,2,3,4,5,6,7,8,9,10,11,12},k;
    int b[]={40,50,60,70,80,90,100};
    int *tmp;
    int len = sizeof(a)/sizeof(int);
    char name[200];

    htable_init(t);
    // fill table
    for(i=0; i<len;i++){
	sprintf(name,"item%d",i);
	htable_add(t,name,(void*)&a[i]);
    }

sprintf(name,"abcdef");
htable_add(t,name,(void*)&b[0]);

sprintf(name,"fedcba");
htable_add(t,name,(void*)&b[1]);

sprintf(name,"bacdef");
htable_add(t,name,(void*)&b[2]);

sprintf(name,"badcef");
htable_add(t,name,(void*)&b[3]);


    //print all items
    printf("Table content rewerse:\n");
    for(i=len-1; i>=0;i--){
	sprintf(name,"item%d",i);
	tmp = (int*)htable_find(t,name);
	printf("%d ",*tmp);
    }


	sprintf(name,"abcdef");
	tmp = (int*)htable_find(t,name);
	printf("%d ",*tmp);

	sprintf(name,"fedcba");
	tmp = (int*)htable_find(t,name);
	printf("%d ",*tmp);
	sprintf(name,"bacdef");
	tmp = (int*)htable_find(t,name);
	printf("%d ",*tmp);

	sprintf(name,"badcef");
	tmp = (int*)htable_find(t,name);
	printf("%d ",*tmp);

    printf("\n");
/*
    for(k=0;k<500;k++){
	i = rand() % len;
	sprintf(name,"item%d",i);
	tmp = (int*)htable_find(t,name);
	printf("%s: %d\n",name,*tmp);
    }
*/
