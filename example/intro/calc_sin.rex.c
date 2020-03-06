/* This file '/home/yoshihiro/omrpc/bin/omrpc-gen' was created by omrpc-gen. Don't edit */

#include "ninf_stub_info.h"
#include "omrpc_stub_lib.h"

short omrpc_stub_version_major = 3;
short omrpc_stub_version_minor = 1;
short omrpc_stub_init = 0;

/* name of module */
char *omrpc_module_name ="calc_sin";

/* number of entry */
short omrpc_n_entry=1;

static char calc_sin_stub_description[] = "";
static struct ninf_param_desc calc_sin_param_desc[] = {
	{ DT_DOUBLE, MODE_IN, 0,},
	{ DT_DOUBLE, MODE_OUT, 1,{
		{ VALUE_CONST, 1, 
			{
			  {VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,},
			  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}
			},
		  VALUE_NONE, 0, 
			{
			  {VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,},
			  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}
			},
		  VALUE_NONE, 0, 
			{
			  {VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,},
			  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}
			},
		  VALUE_NONE, 0, 
			{
			  {VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,},
			  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}
			},
		},
		}},
};
static NINF_STUB_INFO calc_sin_stub_info = {
3,1,0,	"calc_sin","calc_sin",2,
calc_sin_param_desc, 
	0, 
			{
			  {VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,VALUE_NONE,},
			  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,}
			},
	calc_sin_stub_description, 
	0, /* boolean: specify server side shrink */
	0,
	0
};

/* entry name table */
static char *omrpc_entry_name_table[1]={
"calc_sin"
};

NINF_STUB_INFO *omrpc_stub_info_table[1]={
&calc_sin_stub_info
};

/* Globals */
 
#include <math.h>


/* Stub Main program */
int main(int argc, char ** argv){
	int __tmp;

	omrpc_stub_INIT(argc,argv);
	while(1){
	  __tmp = omrpc_stub_REQ();
	  switch(__tmp){
	  default: goto exit;
	  case 0: /* calc_sin */
{
	double d;
	double *result;
		omrpc_stub_SET_ARG(&d,0);
		omrpc_stub_SET_ARG(&result,1);
		omrpc_stub_BEGIN();
{	
    *result = sin(d);
}
		omrpc_stub_END();
}
	  break; 
	   }
	}
exit:
	omrpc_stub_EXIT();
 return 0; }
 /* END OF Stub Main */
