#ifndef EOC_CFG_OPTIONS
#define EOC_CFG_OPTIONS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

#define MAX_OPTSTR 64
#define MAX_STRVAL MAX_OPTSTR
#define MAX_OPTNAME MAX_OPTSTR

typedef struct {
	char strval[MAX_STRVAL];
	int intval;
} option_values;

static option_values tcpam_vals[] = { { "tcpam4",1 },{"tcpam8",2},{"tcpam16",3},{"tcpam32",4},
							{"tcpam64",5},{"tcpam128",6} };
#define tcpam_vsize sizeof(tcpam_vals)/sizeof(option_values)

static option_values annex_vals[] = { { "A",1 },{"B",2}};
#define annex_vsize sizeof(annex_vals)/sizeof(option_values)

static option_values power_vals[] = { { "on",2 },{"off",1}};
#define power_vsize sizeof(power_vals)/sizeof(option_values)
static option_values comp_vals[] = { { "base",0 },{"extended",1}};
#define comp_vsize sizeof(comp_vals)/sizeof(option_values)


class cfg_option{
protected:
	char name[MAX_OPTNAME];
	option_values *vals;
	int valnum;
	int min_val,max_val;
	char sval[MAX_OPTSTR];
	int val;
	int basic;
	unsigned char use_min:1,use_max:1,found:1,valid:1,use_sval:1;

	void init(){
		name[0] = '\0';
		vals = NULL;
		valnum = 0;
		use_min = 0;
		use_max = 0;
		found = 0;
		use_sval = 0;
	}
public:
	typedef enum { Accept, Discard, Exist } chk_res;

	cfg_option(int _basic,const char *_name){
		basic = _basic;
		init();
		use_sval = 1;
		strncpy(name,_name,MAX_OPTNAME);
	}

	cfg_option(int _basic,const char *_name,option_values *opts,int onum){
		basic = _basic;
		init();
		strncpy(name,_name,MAX_OPTNAME);
		valnum = onum;
		vals = opts;
	}

	cfg_option(int _basic,const char *_name,int _use_max,int _max_val,int _use_min,int _min_val){
		basic = _basic;
		init();
		strncpy(name,_name,MAX_OPTNAME);
		use_max = _use_max;
		use_min = _use_min;
		max_val = _max_val;
		min_val = _min_val;
	}

	chk_res chk_option(char *_name,char *_strval){
		if( strncmp(name,_name,strnlen(name,MAX_OPTNAME)) ){
			return Discard;
		}
		if( found ){
			return Exist;
		}
		found = 1;
		// find value of option
		if( use_sval ){
			strncpy(sval,_strval,MAX_OPTSTR);
			valid = 1;
			return Accept;
		}
		if( !valnum ){
			char *endp;
			// this is numeric option
			val = strtol(_strval,&endp,0);
			if( _strval == endp || (endp-_strval) < strnlen(_strval,MAX_STRVAL) ){
				// Wrong string value
				return Accept;
			}
			if( use_max && val > max_val ){
				return Accept;
			}
			if( use_min && val < min_val ){
				return Accept;
			}
			valid = 1;
		}else{
			for(int i=0;i<valnum;i++){
				if( !strncasecmp(_strval,vals[i].strval,strnlen(vals[i].strval,MAX_STRVAL)) ){
					valid = 1;
					val = vals[i].intval;
					return Accept;
				}
			}
		}
		return Accept;
	}

	int is_valid(){ return valid; }
	int is_int(){ return !use_sval; }
	int is_basic() { return basic; }
	int value(){ return val; }
	const char *svalue() { return sval; }
	const char *optname(){ return name; }
};


/*
int
main(int argc, char *argv[])
{
	char mas[]="rate=5696 name=test1 power=off annex=B tcpam=TCPAM128";
	cfg_option prof_opts[] = { cfg_option("name"),cfg_option("tcpam",tcpam_vals,tcpam_vsize),
			cfg_option("annex",annex_vals,annex_vsize),cfg_option("power",power_vals,power_vsize),
			cfg_option("rate",0,0,1,64) };


	char *str1 = mas,*str2;
	char *token,*subtoken;
	char *saveptr1,*saveptr2;
	int j;

	for (j = 1; ; j++, str1 = NULL) {
		token = strtok_r(str1, " ", &saveptr1);
		if (token == NULL)
			break;
		printf("%d: %s\n", j, token);

		char ptrs[2][MAX_OPTSTR];
		int i;
		for (i=0,str2 = token; i<2 ; str2 = NULL,i++) {
			subtoken = strtok_r(str2,"=", &saveptr2);
			if (subtoken == NULL)
				break;
//			printf(" --> %s\n", subtoken);
			strncpy(ptrs[i],subtoken,MAX_OPTSTR);
		}
		printf("%s = %s\n",ptrs[0],ptrs[1]);
		for(i=0;i<sizeof(prof_opts)/sizeof(cfg_option);i++){
			cfg_option::chk_res ret;
			ret = prof_opts[i].chk_option(ptrs[0],ptrs[1]);
			if( ret == cfg_option::Accept || ret == cfg_option::Exist )
				break;
		}
	}

	for(int i=0;i<sizeof(prof_opts)/sizeof(cfg_option);i++){
		if( !prof_opts[i].is_valid() ){
			printf("%s is invalid\n",prof_opts[i].optname());
		}else if(prof_opts[i].is_int() ) {
			printf("%s = %d\n",prof_opts[i].optname(),prof_opts[i].value());
		} else {
			printf("%s = %s\n",prof_opts[i].optname(),prof_opts[i].svalue());
		}
	}



	return 0;
}
*/

#endif
