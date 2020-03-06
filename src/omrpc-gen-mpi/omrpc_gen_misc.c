static char rcsid[] = "$Id: omrpc_gen_misc.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $";
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
#include <stdlib.h>
#include "omrpc_gen.h"

  FILE *print_fp;

void expr_print_rec(expr x, int l);

/* 
 * FOR DEBUG:
 */
void
expr_print(expr x, FILE *fp)
{
    print_fp = fp;
    expr_print_rec(x,0);
    fprintf(print_fp,"\n");
}

/* tree print routine */
void
expr_print_rec(expr x, int l)
{
    int i;
    struct list_node *lp;
    
    /* indent */
    for(i = 0; i < l; i++) fprintf(print_fp,"    ");
    
    if(x == NULL){
      /* special case */
      fprintf(print_fp,"<NULL>");
      return;
    }
    
    fprintf(print_fp,"(%s",EXPR_CODE_NAME(EXPR_CODE(x)));

    fprintf(print_fp,":%d",EXPR_LINENO(x));

    switch (EXPR_CODE(x)){
    case IDENT:
      fprintf(print_fp," \"%s\")",SYM_NAME(EXPR_SYM(x)));
      break;
    case STRING_CONSTANT:
      fprintf(print_fp," \"%s\")",EXPR_STR(x));
      return;
    case INT_CONSTANT:
    case LONG_CONSTANT:
      fprintf(print_fp," %ld)",EXPR_INT(x));
      return;
    case FLOAT_CONSTANT:
      fprintf(print_fp," %g)",EXPR_FLOAT(x));
      return;
    case BASIC_TYPE_NODE:
      fprintf(print_fp," <%s>)",basic_type_name(EXPR_TYPE(x)));
      return;
    case MODE_SPEC_NODE:
      fprintf(print_fp," <%s>)",mode_spec_name(EXPR_MODE_SPEC(x)));
      return;
      
    default:
      /* list */
      if((lp = EXPR_LIST(x)) == NULL){
	fprintf(print_fp,")");
	return;
      }
      for(/* */; lp != NULL; lp = LIST_NEXT(lp)) {
	fprintf(print_fp,"\n");
	expr_print_rec(LIST_ITEM(lp),l+1);
      }
      fprintf(print_fp,")");
      break;
    }
}

char *basic_type_name(DATA_TYPE type)
{
  switch(type){
    
  case DT_VOID: return("VOID");
  case DT_CHAR: return("CHAR");
  case DT_SHORT: return("SHORT");
  case DT_LONG: return("LONG");
  case DT_LONGLONG: return("LONGLONG");
  case DT_UNSIGNED_CHAR: return("UNSIGNED_CHAR");
  case DT_UNSIGNED_SHORT: return("UNSIGNED_CHAR");
  case DT_UNSIGNED_LONG: return("UNSIGNED_LONG");
  case DT_UNSIGNED_LONGLONG: return("UNSIGNED_LONGLONG");
  case DT_FLOAT: return("FLOAT");
  case DT_DOUBLE: return("DOUBLE");
  case DT_LONG_DOUBLE: return("LONG DOUBLE");
  case DT_SCOMPLEX: return("SCOMPLEX");
  case DT_DCOMPLEX: return("DCOMPLEX");
  case DT_UNSIGNED: return("UNSINGED");
  case DT_INT:  return("INT");
  case DT_FILEPOINTER: return("FILEPOINTER");
  case DT_FILENAME: return("FILENAME");
  default:
    return("?unknowntype?");
  }
}

char *mode_spec_name(MODE_SPEC mode)
{
  switch (mode){
  case MODE_NONE: return("mode_none");	/* undefined */
  case MODE_IN: return("mode_in");
  case MODE_OUT: return("mode_out");
  case MODE_INOUT: return("mode_inout");
  default:
    return("?unknownmode?");
  }
}


struct 
{
    enum expr_code code;
    char *name;
} expr_code_info_table[] =
{
	{ERROR_NODE,"ERROR_NODE"},
	{LIST,"LIST"},
	{IDENT,"IDENT"},
	{STRING_CONSTANT,"STRING_CONSTANT"},
	{INT_CONSTANT,"INT_CONSTANT"},
	{LONG_CONSTANT,"LONG_CONSTANT"},
	{FLOAT_CONSTANT,"FLOAT_CONSTANT"},
	{BASIC_TYPE_NODE,"BASIC_TYPE_NODE"},
	{MODE_SPEC_NODE,"MODE_SPEC_NODE"},

	{PLUS_EXPR,"+"},
	{MINUS_EXPR,"-"},
	{UNARY_MINUS_EXPR,"unary-"},
	{MUL_EXPR,"*"},
	{DIV_EXPR,"/"},
	{MOD_EXPR,"%"},
	{BEKI_EXPR,"^"},

	{POINTER_REF,"POINTER_REF"},
	{ARRAY_REF,"ARRAY_REF"},
	{0,NULL}
};

struct expr_code_info expr_code_info[MAX_CODE];

void
init_expr_code_info(void)
{
    int i;
    for (i = 0; expr_code_info_table[i].name != NULL; i++){
      expr_code_info[(int)expr_code_info_table[i].code].code_name =
	expr_code_info_table[i].name;
      expr_code_info[(int)expr_code_info_table[i].code].operator_name =
	expr_code_info_table[i].name; /* same */
    }
}
