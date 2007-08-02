#include <stdio.h>
#include <stdlib.h>
#include <utils/hash_table.h>

class hash_el : public hash_elem{
public:
    int val;
    hash_el(int v){
        val = v;
    }
};
		    


int main()
{
    hash_table t(32);
    int i,k;
    hash_el *a;
    int b[]={40,50,60,70,80,90,100};
    int tmp;
    int len = 10;
    char name[200];
    
    // fill table
    for(i=len-1; i>=0;i--){
	sprintf(name,"item%d",i);
	a = new hash_el(i+1);
	a->name = strdup(name);
	a->nsize = strlen(name);
	t.add(a);
    }

sprintf(name,"abcdef");
a = new hash_el(b[0]);
a->name = strdup(name);
a->nsize = strlen(name);
t.add(a);

sprintf(name,"fedcba");
a = new hash_el(b[1]);
a->name = strdup(name);
a->nsize = strlen(name);
t.add(a);


sprintf(name,"bacdef");
a = new hash_el(b[2]);
a->name = strdup(name);
a->nsize = strlen(name);
t.add(a);

sprintf(name,"badcef");
a = new hash_el(b[3]);
a->name = strdup(name);
a->nsize = strlen(name);
t.add(a);

    //print all items	
    printf("Table content rewerse:\n");
    for(i=len-1; i>=0;i--){
	sprintf(name,"item%d",i);
	tmp = ((hash_el*)t.find(name,strlen(name)))->val;
	printf("%d ",tmp);
    }


	sprintf(name,"abcdef");
	tmp = ((hash_el*)t.find(name,strlen(name)))->val;
	printf("%d ",tmp);

	sprintf(name,"fedcba");
	tmp = ((hash_el*)t.find(name,strlen(name)))->val;	
	printf("%d ",tmp);
	sprintf(name,"bacdef");
	tmp = ((hash_el*)t.find(name,strlen(name)))->val;
	printf("%d ",tmp);

	sprintf(name,"badcef");
	tmp = ((hash_el*)t.find(name,strlen(name)))->val;
	printf("%d ",tmp);
    
    printf("\n");

/*
    for(k=0;k<500;k++){
	i = rand() % len;
	sprintf(name,"item%d",i);
	tmp = ((hash_el*)t->find(name))->val;
	printf("%s: %d\n",name,tmp);
    }
*/    
    printf("\n!!!!!!!!!!!!!!!!!!!!! Try sequential !!!!!!!!!!!!!!!!!!!!\n");
    hash_el *el = (hash_el*) t.first();
    while( el ){
	printf("%s: %d\n",el->name,el->val);
	el = (hash_el*)t.next(el->name,el->nsize);
    } 
    
    t.del("item5",5);
    t.del("item4",5);
    t.del("item0",5);

    printf("\n!!!!!!!!!!!!!!!!!!!!! Try sequential !!!!!!!!!!!!!!!!!!!!\n");
    el = (hash_el*) t.first();
    while( el ){
	printf("%s: %d\n",el->name,el->val);
	el = (hash_el*)t.next(el->name,el->nsize);
    } 

    t.del("item9",5);
    t.del("item8",5);
    t.del("item7",5);
    t.del("item6",5);
    t.del("item3",5);
    t.del("item2",5);
    t.del("item1",5);

    printf("\n!!!!!!!!!!!!!!!!!!!!! Try sequential !!!!!!!!!!!!!!!!!!!!\n");
    el = (hash_el*) t.first();
    while( el ){
	printf("%s: %d\n",el->name,el->val);
	el = (hash_el*)t.next(el->name,el->nsize);
    } 

    t.del("abcdef",6);
    t.del("fedcba",6);
    t.del("bacdef",6);
    t.del("badcef",6);

    printf("\n!!!!!!!!!!!!!!!!!!!!! Try sequential !!!!!!!!!!!!!!!!!!!!\n");
    el = (hash_el*) t.first();
    while( el ){
	printf("%s: %d\n",el->name,el->val);
	el = (hash_el*)t.next(el->name,el->nsize);
    } 
}


