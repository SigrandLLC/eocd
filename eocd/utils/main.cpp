#include <utils/hash_table.h>

/*
 * Hash table test
 */


class span_prof_elem : public hash_elem {
public:
    int i;
    float f;
};


int main()
{
    hash_table tbl(32);
    span_prof_elem *el;
    int i,k;

    int b[]={40,50,60,70,80,90,100};
    int tmp;
    int len = 10;
    char name[200];


//while(1)
{
    // fill table
    for(i=0; i<10;i++){
	el = new span_prof_elem;
	el->name = (char*)malloc(32);
	sprintf(el->name,"item%d",i);
	el->nsize = strlen(el->name);
	el->i = 10 + i;
	el->f = 1.0 + 0.1*i;
	tbl.add((hash_elem*)el);
    }

    el = new span_prof_elem;
    el->name = (char*)malloc(32);
    sprintf(el->name,"abcde",i);
    el->nsize = strlen(el->name);
    el->i = 40;
    el->f = 1.0;
    tbl.add(el);

    el = new span_prof_elem;
    el->name = (char*)malloc(32);
    sprintf(el->name,"bacde",i);
    el->nsize = strlen(el->name);
    el->i = 50;
    el->f = 2.0;
    tbl.add(el);

    el = new span_prof_elem;
    el->name = (char*)malloc(32);
    sprintf(el->name,"cabde",i);
    el->nsize = strlen(el->name);
    el->i = 60;
    el->f = 3.0;
    tbl.add(el);

    el = new span_prof_elem;
    el->name = (char*)malloc(32);
    sprintf(el->name,"abced",i);
    el->nsize = strlen(el->name);
    el->i = 70;
    el->f = 4.0;
    tbl.add(el);
    
    tbl.init_trace();
    span_prof_elem *el;
     
    while( el = (span_prof_elem *)tbl.next_elem() ){
	printf("%s: %d %f\n",el->name,el->i,el->f);
    }
	
/*
    //print all items	
    printf("Table content rewerse:\n");
    for(i=len-1; i>=0;i--){
	sprintf(name,"item%d",i);
	if( el = (span_prof_elem*)tbl.find(name,strlen(name)) )
	    printf("%s: %d %f\n",el->name,el->i,el->f);
    }


    sprintf(name,"abcde");
    if( el = (span_prof_elem*)tbl.find(name,strlen(name)) )
        printf("%s: %d %f\n",el->name,el->i,el->f);

    
    sprintf(name,"bacde");
    if( el = (span_prof_elem*)tbl.find(name,strlen(name)) )
        printf("%s: %d %f\n",el->name,el->i,el->f);
    
    
    sprintf(name,"cabde");
    if( el = (span_prof_elem*)tbl.find(name,strlen(name)) )
        printf("%s: %d %f\n",el->name,el->i,el->f);

    sprintf(name,"abced");
    if( el = (span_prof_elem*)tbl.find(name,strlen(name)) )
        printf("%s: %d %f\n",el->name,el->i,el->f);

/*
    for(k=0;k<500;k++){
	i = rand() % len;
	sprintf(name,"item%d",i);
	if( el = (span_prof_elem*)tbl.find(name,strlen(name)) )
    	    printf("%s: %d %f\n",el->name,el->i,el->f);
    }
*/
    printf("Done\n");    
}
}

