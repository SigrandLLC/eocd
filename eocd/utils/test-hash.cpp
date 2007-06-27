#include <stdio.h>
#include <stdlib.h>
#include "EOCHash_table.h"

class hash_elem : public EOCHash_data{
public:
    int val;
    hash_elem(int v){
        val = v;
    }
};
		    


int main()
{
    EOCHash_table *t = new EOCHash_table;
    int i,k;
    hash_elem *a;
    int b[]={40,50,60,70,80,90,100};
    int tmp;
    int len = 10;
    char name[200];
    
    // fill table
    for(i=0; i<len;i++){
	sprintf(name,"item%d",i);
	a = new hash_elem(i+1);
	t->add(name,sizeof(name),a);
    }

sprintf(name,"abcdef");
a = new hash_elem(b[0]);
t->add(name,sizeof(name),a);

sprintf(name,"fedcba");
a = new hash_elem(b[1]);
t->add(name,sizeof(name),a);


sprintf(name,"bacdef");
a = new hash_elem(b[2]);
t->add(name,sizeof(name),a);

sprintf(name,"badcef");
a = new hash_elem(b[3]);
t->add(name,sizeof(name),a);

    //print all items	
    printf("Table content rewerse:\n");
    for(i=len-1; i>=0;i--){
	sprintf(name,"item%d",i);
	tmp = ((hash_elem*)t->find(name))->val;
	printf("%d ",tmp);
    }


	sprintf(name,"abcdef");
	tmp = ((hash_elem*)t->find(name))->val;
	printf("%d ",tmp);

	sprintf(name,"fedcba");
	tmp = ((hash_elem*)t->find(name))->val;	
	printf("%d ",tmp);
	sprintf(name,"bacdef");
	tmp = ((hash_elem*)t->find(name))->val;
	printf("%d ",tmp);

	sprintf(name,"badcef");
	tmp = ((hash_elem*)t->find(name))->val;
	printf("%d ",tmp);
    
    printf("\n");


    for(k=0;k<500;k++){
	i = rand() % len;
	sprintf(name,"item%d",i);
	tmp = ((hash_elem*)t->find(name))->val;
	printf("%s: %d\n",name,tmp);
    }
    
}


