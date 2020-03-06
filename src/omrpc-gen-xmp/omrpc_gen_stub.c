static char rcsid[] = "$Id: omrpc_gen_stub.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $";
/* 
 * $Release: omnirpc-2.0.1 $
 * $Copyright:
 *  OmniRPC Version 1.0
 *  Copyright (C) 2002-2004 HPCS Laboratory, University of Tsukuba.
 *  
 *  This software is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License version
 *  2.1 published by the Free Software Foundation.
 *  
 *  Please check the Copyright and License information in the files named
 *  COPYRIGHT and LICENSE under the top  directory of the OmniRPC Grid PRC 
 *  System release kit.
 *  
 *  
 *  $
 */
/*
 *  stub wrapper generation
 *
 *  $Id: omrpc_gen_stub.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $
 */
/* 
 * modifiled for OmniRPC
 */
#include <string.h>
#include <ctype.h>
#include "omrpc_gen.h"

#define FORTRAN "fortran"

#define GET_VAL_STRING(i) value_type_string[(i) + 1]
static char * value_type_string[] = {
  "VALUE_ERROR",
  "VALUE_NONE",
  "VALUE_CONST",
  "VALUE_IN_ARG",
  "VALUE_BY_EXPR",
  "VALUE_OP",
  "VALUE_END_OF_OP",
  "VALUE_IN_HEADER",
};

#define GET_MODE_STRING(i) mode_type_string[(i)]
static char * mode_type_string[] = {
  "MODE_NONE",
  "MODE_IN",
  "MODE_OUT",
  "MODE_INOUT",
  "MODE_WORK",
  "MODE_DUMMY",
  "MODE_DUMMY",
  "MODE_DUMMY",
  "MODE_CALL_BACK_FUNC"
};

#define GET_DATA_STRING(i) data_type_string[(i)]
static char * data_type_string[] = {
    "DT_UNDEF",
    "DT_VOID",
    "DT_CHAR",
    "DT_SHORT",
    "DT_INT",
    "DT_LONG",
    "DT_LONGLONG",
    "DT_UNSIGNED_CHAR",
    "DT_UNSIGNED_SHORT",
    "DT_UNSIGNED",
    "DT_UNSIGNED_LONG",
    "DT_UNSIGNED_LONGLONG",
    "DT_FLOAT",
    "DT_DOUBLE",
    "DT_LONG_DOUBLE",
    "DT_SCOMPLEX",
    "DT_DCOMPLEX",
    "DT_STRING_TYPE",
    "DT_FILENAME",
    "DT_FILEPOINTER",
    "DT_FUNC_TYPE",
    "BASIC_TYPE_END",
};

extern char * current_fortran_format;

/**  function declarations  **/

static char *C_type_name(DATA_TYPE type);
static int nead_and(char * name, struct stub_gen_entry *ep);
static int nead_and_sub(expr body_expr, DATA_TYPE type, int ndim, enum mode_spec param_inout);
static char *make_function_name(expr body_expr, char * fformat);

static void generate_stub_info(FILE * fp, struct stub_gen_entry * ep,int index);
static void generate_stub_prog(FILE *fp, struct stub_gen_entry *ep);
static void generate_stub_param_desc(FILE * fp, struct stub_gen_entry * ep);
static void ninf_expression_print(FILE * fp, NINF_EXPRESSION * exp);

/* generate RPC execuables */
void 
generate_rpc_executable(FILE *fp)
{
    int i;

    fprintf(fp,"/* This file '%s' was created by omrpc-gen. Don't edit */\n\n",
	    program);
    fprintf(fp,"#include \"ninf_stub_info.h\"\n");
    fprintf(fp,"#include \"omrpc_stub_lib.h\"\n");
    fprintf(fp,"#include <unistd.h>\n");
    fprintf(fp,"\n");
    fprintf(fp,"#include <xmp.h>\n");
    fprintf(fp,"#include <mpi.h>\n");
    fprintf(fp,"\n");
    fprintf(fp,"#ifndef _XMP_HEADER_H_\n");
    fprintf(fp,"extern void xmp_start_rex(int argc, char **argv);\n");
    fprintf(fp,"extern void xmp_finish_rex();\n");
    fprintf(fp,"#endif\n");
    fprintf(fp,"\n");

    fprintf(fp,"short omrpc_stub_version_major = %d;\n",MAJOR_VERSION);
    fprintf(fp,"short omrpc_stub_version_minor = %d;\n",MINOR_VERSION);
    fprintf(fp,"short omrpc_stub_init = %d;\n",0);
    fprintf(fp,"\n");

    /* module name */
    fprintf(fp,"/* name of module */\n");
    if(current_module) 
	fprintf(fp,"char *omrpc_module_name =\"%s\";\n\n",current_module);
    else fprintf(fp,"char *omrpc_module_name =\"\";\n\n");

    fprintf(fp,"/* number of entry */\nshort omrpc_n_entry=%d;\n\n",
	    n_stubs);
    
    /* stub information */
    for(i = 0; i < n_stubs; i++)
	generate_stub_info(fp,&stubs[i],i);
	
    /* entry name table */
    fprintf(fp,"/* entry name table */\n");
    fprintf(fp,"static char *omrpc_entry_name_table[%d]={\n",n_stubs);
    for(i = 0; i < n_stubs; i++){
	fprintf(fp,"\"%s\"", SYM_NAME(stubs[i].ent_id));
	if(i != (n_stubs-1)) fprintf(fp,",\n");
    }
    fprintf(fp,"\n};\n\n");

    /* stub_info table */
    fprintf(fp,"NINF_STUB_INFO *omrpc_stub_info_table[%d]={\n",n_stubs);
    for(i = 0; i < n_stubs; i++){
	fprintf(fp,"&%s_stub_info", SYM_NAME(stubs[i].ent_id));
	if(i != (n_stubs-1)) fprintf(fp,",\n");
    }
    fprintf(fp,"\n};\n\n");
    
    /* generate globals */
    fprintf(fp,"/* Globals */\n");
    for (i = 0; i < n_globals; i++) fprintf(fp,"%s\n",globals[i]);

    /* generate programs */
    /* generate main program */
    fprintf(fp,"\n/* Stub Main program */\nint main(int argc, char ** argv){\n");
    fprintf(fp, "\tint __tmp,__myrank;\n");

    //fprintf(fp,"\tchar nodename[256];\n");
    //fprintf(fp,"\tgethostname(nodename,256);\n");
    //fprintf(fp,"\tprintf(\"nodename = %c%c\\n\",nodename);\n",37,115);

    fprintf(fp,"\t_XMP_init(argc, argv);\n");
    fprintf(fp,"\txmp_start_rex(argc,argv);\n");
    fprintf(fp,"\tif(REX_COMM_WORLD==MPI_COMM_NULL)\n");
#ifdef MYX
    fprintf(fp,"\t\tgetMySetComm(&REX_COMM_WORLD);\n");
#else
    fprintf(fp,"\t\tMPI_Comm_dup(MPI_COMM_WORLD, &REX_COMM_WORLD);\n"); 
#endif
    fprintf(fp,"\tMPI_Comm_rank(REX_COMM_WORLD,&__myrank);\n");
    fprintf(fp,"\n\tif(__myrank==0) omrpc_stub_INIT(argc,argv);\n");
    fprintf(fp,"\twhile(1){\n");
    fprintf(fp,"\t  if(__myrank==0) __tmp = omrpc_stub_REQ();\n");    
    fprintf(fp,"\t  MPI_Bcast(&__tmp,1,MPI_INT,0,REX_COMM_WORLD);\n");
    fprintf(fp,"\t  switch(__tmp){\n");
    fprintf(fp,"\t  default: goto exit;\n");
    for(i = 0; i < n_stubs; i++){
	fprintf(fp,"\t  case %d: // %s \n",i,SYM_NAME(stubs[i].ent_id));
	generate_stub_prog(fp,&stubs[i]);
	fprintf(fp,"\t  break; \n");
    }
    fprintf(fp,"\t   } // switch \n");
    fprintf(fp,"\t} // while \nexit:\n");
    fprintf(fp,"  if(__myrank==0) omrpc_stub_EXIT_MPI();\n");
    fprintf(fp,"  MPI_Barrier(REX_COMM_WORLD);");
    fprintf(fp,"  xmp_finish_rex();\n");
    fprintf(fp,"  _XMP_finalize();\n");
    fprintf(fp," return 0; }\n /* END OF Stub Main */\n"); 
}

void
generate_stub_info(FILE *fp,struct stub_gen_entry *ep, int index)
{
    char *ent_name;
    int info_type = MAT_STYLE_C;
    
    ent_name = SYM_NAME(ep->ent_id);
    /* output description */
    if (ep->description) {
      fprintf(fp,"static char %s_stub_description[] = \"%s\";\n", 
	      ent_name,ep->description);
    } else {
      fprintf(fp,"static char %s_stub_description[] = \"\";\n",
	      ent_name);
    }

    if (ep->language != NULL && strcasecmp(ep->language, FORTRAN) == 0)
	info_type = MAT_STYLE_FORTRAN;
    else if (ep->body_expr)
	if (EXPR_ARG1(ep->body_expr) != NULL) 
	    if (strcasecmp(EXPR_STR(EXPR_ARG1(ep->body_expr)), FORTRAN) == 0)
		info_type = MAT_STYLE_FORTRAN;
    
    generate_stub_param_desc(fp, ep);
    
    fprintf(fp,"static NINF_STUB_INFO %s_stub_info = {\n",ent_name);
    fprintf(fp,"%d,%d,%d,",MAJOR_VERSION, MINOR_VERSION, info_type);
    fprintf(fp,"\t\"%s\",\"%s\",%d,\n",
	    current_module == NULL ? "": current_module,
	    ent_name,ep->nparam);
    fprintf(fp, "%s_param_desc, \n",ent_name);
    fprintf(fp, "\t%d, \n", ep->order_type);
    ninf_expression_print(fp, &(ep->order));
    fprintf(fp, "\t%s_stub_description, \n",ent_name);
    fprintf(fp, "\t%d, /* boolean: specify server side shrink */\n", 
	    ep->shrink);
    fprintf(fp, "\t%d,\n", ep->backend);
    fprintf(fp, "\t%d\n",index);
    fprintf(fp,"};\n\n");
}


void
ninf_expression_print(FILE * fp, NINF_EXPRESSION * exp)
{
  int i;
  fprintf(fp, "\t\t\t{\n");
  fprintf(fp, "\t\t\t  {");    
  for (i = 0; i < NINF_EXPRESSION_LENGTH; i++)
    fprintf(fp, "%s,", GET_VAL_STRING(exp->type[i]));
  fprintf(fp, "},\n");  
  fprintf(fp, "\t\t\t  {");    
  for (i = 0; i < NINF_EXPRESSION_LENGTH; i++)
    fprintf(fp, "%d,", exp->val[i]);
  fprintf(fp, "}\n");  
  fprintf(fp, "\t\t\t},\n");
}

void
generate_stub_param_desc(FILE * fp, struct stub_gen_entry * ep)
{
  struct param_gen_desc *dp;
  int i,j;

  fprintf(fp,"static struct ninf_param_desc %s_param_desc[] = {\n",
	  SYM_NAME(ep->ent_id));
  for (i = 0; i < ep->nparam; i++){
    dp = &ep->params[i];
    fprintf(fp,"\t{ %s, %s, %d,",GET_DATA_STRING(dp->param_type),
	    GET_MODE_STRING(dp->param_inout),dp->ndim);
    if (dp->ndim > 0){
      fprintf(fp,"{\n");
      for (j = 0; j < dp->ndim; j++){
	fprintf(fp,"\t\t{ %s, %d, \n",
		GET_VAL_STRING(dp->dim[j].size_type), dp->dim[j].size);
	ninf_expression_print(fp, &(dp->dim[j].size_exp));
	fprintf(fp,"\t\t  %s, %d, \n",
		GET_VAL_STRING(dp->dim[j].start_type), dp->dim[j].start);
	ninf_expression_print(fp, &(dp->dim[j].start_exp));
	fprintf(fp,"\t\t  %s, %d, \n",
		GET_VAL_STRING(dp->dim[j].end_type), dp->dim[j].end);
	ninf_expression_print(fp, &(dp->dim[j].end_exp));
	fprintf(fp,"\t\t  %s, %d, \n",
		GET_VAL_STRING(dp->dim[j].step_type), dp->dim[j].step);
	ninf_expression_print(fp, &(dp->dim[j].step_exp));
	fprintf(fp,"\t\t},\n");
      }
      fprintf(fp,"\t\t}");
    }
    fprintf(fp,"},\n");
  }
  fprintf(fp,"};\n");
}

void
generate_stub_body(struct stub_gen_entry *ep, FILE * fp){
    list lp;

    if (ep->body_expr){
      /* generate body */
      char * function_name;
      function_name = make_function_name(ep->body_expr, current_fortran_format);
      fprintf(fp,"\t\t%s(", function_name);


      FOR_ITEMS_IN_LIST(lp,EXPR_ARG3(ep->body_expr)){
	char * tmp = SYM_NAME(EXPR_SYM(LIST_ITEM(lp)));
	fprintf(fp,"%s%s", nead_and(tmp, ep)?"&":"", tmp);
	if(LIST_NEXT(lp) != NULL) 
	  fprintf(fp,",");
      }
      fprintf(fp,");\n");
    } else 
      fprintf(fp,"%s\n",ep->body);
}

void
generate_stub_prog(FILE *fp,struct stub_gen_entry *ep)
{
    char *ent_name;
    struct param_gen_desc *dp;
    int i;
    
    ent_name = SYM_NAME(ep->ent_id);
    fprintf(fp,"{\n");
    for (i = 0; i < ep->nparam; i++){
      dp = &ep->params[i];
      fprintf(fp, "\t%s %s%s;\n", C_type_name(dp->param_type),
	      (dp->ndim > 0) ? "*":"",
	      SYM_NAME(dp->param_id));
    }

    /* input and setup working area */

    fprintf(fp,"\t\tif(__myrank==0){\n");
    for (i = 0; i < ep->nparam; i++){
      dp = &ep->params[i];
      fprintf(fp,"\t\tomrpc_stub_SET_ARG(&%s,%d);\n",SYM_NAME(dp->param_id),i);
    }

    fprintf(fp,"\t\tomrpc_stub_BEGIN();\n");
    fprintf(fp,"\t\t}\n");


    /* body */
    generate_stub_body(ep, fp);
    
    fprintf(fp,"\t\tif(__myrank==0) omrpc_stub_END();\n");
    fprintf(fp,"\t\t}\n");
    
}

static char function_name_buffer[100];

char * fortran_format(char * buf, char * fformat, char * fname)
{
  char * answer = buf;
  if (fformat == NULL)
    return fname;
  while (*fformat != '\0'){
    if (*fformat != '%'){
      *buf++ = *fformat++;
      continue;
    } else {
      char * tmp;
      switch (*(++fformat)){
      case 's': 
      case 'S': 
	tmp = fname;
	while (*tmp != '\0')
	  *buf++ = *tmp++;
	break;
      case 'l': 
      case 'L': 
	tmp = fname;
	while (*tmp != '\0'){
	  char tc = *tmp++;
	  *buf++ = toupper(tc);
	}
	break;
      default:
	fprintf(stderr, "unknown fortran format %%%c: ignore\n", *fformat);
      }
      fformat++;
    }   
  }
  *buf = '\0';
  return answer;
}


static char *
make_function_name(expr body_expr, char * fformat)
{
  if (EXPR_ARG1(body_expr) != NULL) 
    if (strcasecmp(EXPR_STR(EXPR_ARG1(body_expr)), FORTRAN) == 0){
      return fortran_format(function_name_buffer, fformat ,
			    SYM_NAME(EXPR_SYM(EXPR_ARG2(body_expr))));
  } 
  return SYM_NAME(EXPR_SYM(EXPR_ARG2(body_expr)));
}

static int
nead_and(char * name, struct stub_gen_entry *ep){
  int i;
  struct param_gen_desc *dp;
  for (i = 0; i < ep->nparam; i++){
    dp = &ep->params[i];
    if (strcmp(SYM_NAME(dp->param_id), name) != 0)
      continue;
    if (dp->param_type == DT_FUNC_TYPE)
      break;
    return nead_and_sub(ep->body_expr,dp->param_type, dp->ndim, dp->param_inout);
  }
  return 0;
}

static int
nead_and_sub(expr body_expr, DATA_TYPE type, int ndim, enum mode_spec param_inout)
{
  if (EXPR_ARG1(body_expr) == NULL) 
    return 0;
  if (strcasecmp(EXPR_STR(EXPR_ARG1(body_expr)), FORTRAN) != 0)
    return 0;
  if (ndim != 0)
    return 0;
  if ((param_inout & MODE_IN) == 0)
    return 0;
  if (type == DT_STRING_TYPE)
      return 0;
  if (type == DT_FILENAME)
      return 0;
  return 1;
}

static char *C_type_name(DATA_TYPE type)
{
  switch (type){
  case DT_VOID: return("void");
  case DT_CHAR: return("char");
  case DT_SHORT: return("short");
  case DT_LONG: return("long");
  case DT_LONGLONG: return("long long");
  case DT_UNSIGNED_CHAR: return("unsigned char");
  case DT_UNSIGNED_SHORT: return("unsigned short");
  case DT_UNSIGNED_LONG: return("unsigned long");
  case DT_UNSIGNED_LONGLONG: return("unsigned long long");
  case DT_FLOAT: return("float");
  case DT_DOUBLE: return("double");
  case DT_LONG_DOUBLE: return("long double");
  case DT_STRING_TYPE: return("char *");    
  case DT_UNSIGNED: return("unsinged");
  case DT_INT:  return("int");
  case DT_SCOMPLEX: return("float complex");
  case DT_DCOMPLEX: return("double complex");    
  case DT_FILENAME: return("char *");
  case DT_FILEPOINTER: return("FILE *");
  default:
    fatal("unknown type name %d\n",type);
    return("unknown");
  }
}



