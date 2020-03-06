static char rcsid[] = "$Id: omrpc_gen_compile.c,v 1.1.1.1 2004-11-03 21:01:21 yoshihiro Exp $";
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
#include <string.h>
#include "omrpc_gen.h"
#include "y.tab.dummy.h"
#include "y.tab.h"

struct stub_gen_entry stubs[MAX_STUBS];
int n_stubs = 0;
struct stub_gen_entry *current_stub;

char *current_module = NULL;
char *current_fortran_format = NULL;

char *globals[MAX_GLOBALS];
int n_globals = 0;

/**  function declarations  **/

static DATA_TYPE expr_to_type(expr x);
static DATA_TYPE combine_type(expr x, DATA_TYPE t2);

static int compile_param(struct param_gen_desc *dp, expr param);
static int compile_param_decl(struct param_gen_desc *dp, int dim, expr decl);
static VALUE_TYPE compile_param_value_front(struct stub_gen_entry *ep, 
					    expr x, int * v, NINF_EXPRESSION *expression);
static VALUE_TYPE compile_param_value(struct stub_gen_entry * ep,
				      expr x, int *v, NINF_EXPRESSION * expression,
				      int *exp_index);
void
compile_MODULE(expr x)
{
  if (debug_flag) {
    fprintf(stderr, "Module:");
    expr_print(x,stderr);
    fprintf(stderr, "\n");
  }
  if (current_module != NULL){
    error_at_node(x,"module name is already defined");
    return;
  }
  if(EXPR_CODE(x) == IDENT)
    current_module = SYM_NAME(EXPR_SYM(x));
  else if(EXPR_CODE(x) == STRING_CONSTANT)
    current_module = EXPR_STR(x);
  else 
    error_at_node(x,"illegal module name");
}

void
compile_FORTRANFORMAT(expr x)
{
  if(EXPR_CODE(x) != STRING_CONSTANT) fatal("FORTRANFORMAT");
  current_fortran_format = EXPR_STR(x);
}

void
compile_COMPILE_OPTIONS(expr x)
{
    warning("'CompileOptions' is not supported, ignored");
}

void
compile_GLOBALS(expr x)
{
  if (debug_flag){
      fprintf(stderr, "Globals:");
      expr_print(x,stderr);
      fprintf(stderr, "\n");
  }
  if (EXPR_CODE(x) != STRING_CONSTANT) fatal("GLBOALS");
  if (n_globals >= MAX_GLOBALS) fatal("too many Globals");
  globals[n_globals++] = EXPR_STR(x);
}

expr compile_required(expr x){
  return list2(LIST, make_enode(INT_CONSTANT, REQUIRED), x);
}
expr compile_backend(expr x){
  return list2(LIST, make_enode(INT_CONSTANT, BACKEND), x);
}
expr compile_shrink(expr x){
  return list2(LIST, make_enode(INT_CONSTANT, SHRINK), x);
}
expr compile_language(expr x){
  return list2(LIST, make_enode(INT_CONSTANT, LANGUAGE), x);
}
expr compile_calcorder(expr x){
  return list2(LIST, make_enode(INT_CONSTANT, CALCORDER), x);
}

void
compile_LIBRARY(expr x)
{
    warning("'Library' is not supported, ignored");
}

void
compile_interface_getoption(expr option_list, expr * backendp, expr * reqp,
			     expr * orderp, expr * languagep, expr * shrinkp){
  struct list_node *lp;
  if (option_list == NULL) return;
  lp = EXPR_LIST(option_list);
  while (lp != NULL){
    expr tmp = LIST_ITEM(lp);
    if (EXPR_CODE(tmp) != LIST)
      error_at_node(tmp, "internal error: expects list");
    if (EXPR_CODE(EXPR_ARG1(tmp)) != INT_CONSTANT)
      error_at_node(tmp, "internal error: expects int");
    switch (EXPR_INT(EXPR_ARG1(tmp))){
    case REQUIRED:
      *reqp = EXPR_ARG2(tmp); break;
    case BACKEND:
      *backendp = EXPR_ARG2(tmp); break;
    case CALCORDER:
      *orderp = EXPR_ARG2(tmp); break;
    case LANGUAGE:
      *languagep = EXPR_ARG2(tmp); break;
    case SHRINK:
      *shrinkp = EXPR_ARG2(tmp); break;
    default:
      error_at_node(tmp, "internal error: unknown code");
    }
    lp = LIST_NEXT(lp);
  }
}


void
compile_interface(expr def, expr desc, expr option_list, expr body)
{
    list lp;
    int param_count;
    struct stub_gen_entry *ep;
    struct param_gen_desc *dp;
    struct dim_gen_desc *dimp;
    int i, k;
    int dum;
    NINF_EXPRESSION * order_p;
    expr x;
    expr backend = NULL;
    expr req = NULL;
    expr order = NULL;
    expr language = NULL;
    expr shrink = NULL;
      
    compile_interface_getoption(option_list, &backend, &req, &order, 
				             &language, &shrink);
    if (debug_flag){
	fprintf(stderr, "Define:");
	expr_print(def,stderr);
	fprintf(stderr, "\n\tDesc:");
	expr_print(desc,stderr);
	fprintf(stderr, "\n\trequired:");
	expr_print(req,stderr);
	fprintf(stderr, "\n\tbody:");
	expr_print(body,stderr);
	fprintf(stderr, "\n");
    }
    if (n_stubs >= MAX_STUBS) fatal("too many interface definition");
    ep = &stubs[n_stubs++];	/* get entry */

    /* def = (LIST ident (LIST param1 ...)) */
    if (EXPR_CODE(EXPR_ARG1(def)) != IDENT) fatal("Define: not ident");
    ep->ent_id = EXPR_SYM(EXPR_ARG1(def));

    param_count = 0;
    FOR_ITEMS_IN_LIST(lp,EXPR_ARG2(def)){
      if (param_count >= MAX_PARAMS){
	error_at_node(def,"too many paramters");
	break;
      }
      param_count += compile_param(&ep->params[param_count],LIST_ITEM(lp));
    }
    ep->nparam = param_count;
    
    if (desc != NULL) ep->description = EXPR_STR(desc);
    if (req != NULL) ep->required = EXPR_STR(req);
    if (language != NULL) ep->language = EXPR_STR(language);
    else ep->language = NULL;

    if (EXPR_CODE(body) == STRING_CONSTANT)
      ep->body = EXPR_STR(body);
    else ep->body_expr = body;

    /* check paramter */
    for (i = 0; i < param_count; i++){
      dp = &ep->params[i];
      if(dp->param_id == NULL) continue;  /* error recovery */
      /* check OUT paramter. OUT paramter must be indirect. */
      /* except filepointer type */
      if(IS_OUT_MODE(dp->param_inout) && dp->ndim == 0){// &&
//         dp->param_type != DT_FILEPOINTER &&
//         dp->param_type != DT_FILENAME ){
          error_at_node(def,"scalar out parameter, %s",
                        SYM_NAME(dp->param_id));
          continue;
      }
      /* check dimension paramter. dimension must be CONSTANT or 
	 IN (scalar) paramter */
      /* !!! NOTE: size calcuation is not allowed in this version!!! */
      /* !!! Size calcuration is Supported by H.N. */
      for(k = 0; k < dp->ndim; k++){
	dimp = &dp->dim[k];
	if(dimp->size_type != VALUE_NONE){
	  /* already set by someone */
	  continue;
	}
	dimp->size_type = 
	  compile_param_value_front(ep,dimp->size_expr,&dimp->size, 
				    &dimp->size_exp);
	if(dimp->size_type == VALUE_NONE){
	  if(k != 0)
	    error_at_node(def,"NULL dimension must be first, %s",
			  SYM_NAME(dp->param_id));
	  dimp->size_type = VALUE_CONST;
	  dimp->size = 1;
	} else if(dimp->size_type == VALUE_CONST && dimp->size <= 0)
	  error_at_node(def,"dimension must be positive, %s",
			SYM_NAME(dp->param_id));
	/* process the range expressions */
	x = dimp->range_exprs;
	if(x != NULL){
	  dimp->start_type =
	    compile_param_value_front(ep,EXPR_ARG1(x),&dimp->start,
				      &dimp->start_exp);
	  dimp->end_type =
	    compile_param_value_front(ep,EXPR_ARG2(x),&dimp->end,
				      &dimp->end_exp);
	  dimp->step_type =
	    compile_param_value_front(ep,EXPR_ARG3(x),&dimp->step,
				      &dimp->step_exp);
	}
      }
    }

    if (backend != NULL){
      if ((strncasecmp(EXPR_STR(backend), "mpi", 4))== 0)
	ep->backend = BACKEND_MPI;
      else
	error_at_node(backend, "unsupported backend");
    } else
      ep->backend = BACKEND_NORMAL;

    if (order != NULL){
      ep->order_type = VALUE_BY_EXPR;
      order_p = &(ep->order);
      compile_param_value_front(ep, order, &dum, order_p);
    } else {
      ep->order_type = VALUE_NONE;
    }

    if (shrink != NULL &&
	strncasecmp(EXPR_STR(shrink), "yes", 4) == 0)
      ep->shrink = TRUE;
    else
      ep->shrink = FALSE;
}

void
push_expression(int type, int val, NINF_EXPRESSION * expression, int * exp_index)
{
  expression->type[*exp_index] = type;
  expression->val [*exp_index] = val;
  (*exp_index)++;
}

VALUE_TYPE compile_param_value_front(struct stub_gen_entry *ep, 
				     expr x, int * v, NINF_EXPRESSION *expression)
{
  int exp_index = 0;
  VALUE_TYPE tmp_t = compile_param_value(ep,x,v,expression, &exp_index);
  if (tmp_t == VALUE_OP){
    push_expression(tmp_t, *v, expression, &exp_index);
    push_expression(VALUE_END_OF_OP, 0, expression, &exp_index);
    return VALUE_BY_EXPR;
  } 
  return tmp_t;
}

VALUE_TYPE compile_param_value(struct stub_gen_entry * ep,
			       expr x, int *v, NINF_EXPRESSION * expression,
			       int *exp_index)
{
  int j;
  int v1, v2, v3, t1, t2, t3;
  
  if(x == NULL) return(VALUE_NONE);	/* none */
  
  switch (EXPR_CODE(x)){
  case INT_CONSTANT:
    *v = EXPR_INT(x);
    return(VALUE_CONST);
  case IDENT:
    /* search IN scalar paramter */
    for(j = 0; j < ep->nparam; j++)
      if(ep->params[j].param_id == EXPR_SYM(x)) break;
    if(j == ep->nparam)
      error_at_node(x,"input parameter is not found, %s",
		    SYM_NAME(EXPR_SYM(x)));
    else if(ep->params[j].ndim == 0 &&
	    IS_IN_MODE(ep->params[j].param_inout))
      {
	*v = j;
	return(VALUE_IN_ARG);
      }
    else
      error_at_node(x,"parameter in expression must be IN scalar, %s",
		    SYM_NAME(EXPR_SYM(x)));
    return(VALUE_ERROR);
    /*break;*/

  case PLUS_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_PLUS;
    return VALUE_OP;

  case MINUS_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_MINUS;
    return VALUE_OP;

  case MUL_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_MUL;
    return VALUE_OP;

  case BEKI_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_BEKI;
    return VALUE_OP;


  case DIV_EXPR:      
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_DIV;
    return VALUE_OP;

  case MOD_EXPR:      
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_MOD;
    return VALUE_OP;
    
  case EQ_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_EQ;
    return VALUE_OP;
  case NEQ_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_NEQ;
    return VALUE_OP;
  case GT_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_GT;
    return VALUE_OP;
  case LT_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_LT;
    return VALUE_OP;
  case GE_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_GE;
    return VALUE_OP;
  case LE_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    *v = OP_VALUE_LE;
    return VALUE_OP;
  case TRY_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    t2 = compile_param_value(ep,EXPR_ARG2(x), &v2,expression, exp_index) ;
    push_expression(t2, v2, expression, exp_index);      
    t3 = compile_param_value(ep,EXPR_ARG3(x), &v3,expression, exp_index) ;
    push_expression(t3, v3, expression, exp_index);      
    *v = OP_VALUE_TRY;
    return VALUE_OP;
  case UNARY_MINUS_EXPR:
    t1 = compile_param_value(ep,EXPR_ARG1(x), &v1,expression, exp_index) ;
    push_expression(t1, v1, expression, exp_index);      
    *v = OP_VALUE_UN_MINUS;
    return VALUE_OP;

  default:
    error_at_node(x,"expression is not supported yet, sorry!");
    /* expression is not yet */
    return(VALUE_ERROR);
  }
}

int compile_callback(struct param_gen_desc *dp, expr param)
{
  list lp;
  int index = 1;
  dp->param_type = DT_FUNC_TYPE;
  dp->param_inout = MODE_CALL_BACK_FUNC;
  dp->param_id = EXPR_SYM(EXPR_ARG1(param));
  FOR_ITEMS_IN_LIST(lp, EXPR_ARG2(param)){
    index++;
    compile_param(++dp, LIST_ITEM(lp));
  }
  return index;
}

/* param = (LIST (LIST mode type) decl) */
/* returns parameter counts: usualy returns 1.
              for callback function, returns param+1  */
static int compile_param(struct param_gen_desc *dp, expr param)
{     
    expr mode,type;

    if(param == NULL) return 0;
    if (EXPR_CODE(param) == CALLBACK_FUNC){
      return compile_callback(dp, param);
    }
    mode = EXPR_ARG1(EXPR_ARG1(param));
    if(mode != NULL) dp->param_inout = EXPR_MODE_SPEC(mode);
    type = EXPR_ARG2(EXPR_ARG1(param));
    if(type == NULL) error_at_node(param,"no parameter type");
    else dp->param_type = expr_to_type(type);
    dp->ndim = compile_param_decl(dp,0,EXPR_ARG2(param));
    return 1;
}

/* decl := ident | (ARRAY_REF expr range)) */
/* return the dimension */
static int
compile_param_decl(struct param_gen_desc *dp, int dim, expr decl)
{
    switch(EXPR_CODE(decl)){
    case IDENT:
      dp->param_id = EXPR_SYM(decl);
      return(dim);
    case ARRAY_REF:
      dp->dim[dim].size_expr = EXPR_ARG2(decl);
      dp->dim[dim].range_exprs = EXPR_ARG3(decl);
      return(compile_param_decl(dp,dim+1,EXPR_ARG1(decl)));
    case POINTER_REF:	
      /* POINTER_REF is same as size 1 array */
      dp->dim[dim].size_type = VALUE_CONST;
      dp->dim[dim].size = 1;
      if(EXPR_CODE(EXPR_ARG1(decl)) != IDENT)
	error_at_node(decl,"bad pointer declaration");
      return(compile_param_decl(dp,dim+1,EXPR_ARG1(decl)));
      
      /* !!! NOTE: other declaration is not implemented yet !!! */
    default:
      error_at_node(decl,"unkonwn declarator");
      return(0);
    }
}

static DATA_TYPE expr_to_type(expr x)
{
  if (EXPR_CODE(x) == BASIC_TYPE_NODE)
    return(EXPR_TYPE(x));
  else if (EXPR_CODE(x) == LIST)
    return(combine_type(EXPR_ARG1(x),expr_to_type(EXPR_ARG2(x))));
  else fatal("illegal type");
  return DT_UNDEF;
}

static DATA_TYPE combine_type(expr x, DATA_TYPE t2)
{
  DATA_TYPE t1,t;
  
  if (EXPR_CODE(x) != BASIC_TYPE_NODE) goto err;
  t1 = EXPR_TYPE(x);
  t = DT_UNDEF;
  if (t1 == DT_UNDEF || t2 == DT_UNDEF) goto ok;	/* don't care */
  
  switch(t1){
  default: 
    goto err;
  case DT_UNSIGNED:
    switch(t2){
    case DT_CHAR: t = DT_UNSIGNED_CHAR; goto ok;
    case DT_INT: t = DT_UNSIGNED; goto ok;
    case DT_SHORT:	t = DT_UNSIGNED_SHORT; goto ok;
    case DT_LONG: t = DT_UNSIGNED_LONG; goto ok;
    default: { break; }
    }
    break;
  case DT_LONG:	/* size spec */
    switch(t2){
    case DT_UNSIGNED: t = DT_UNSIGNED_LONG; goto ok;
    case DT_FLOAT:	t = DT_DOUBLE; goto ok;
    case DT_INT: t = DT_LONG; goto ok;
    default: { break; }
    }
    break;
  case DT_SHORT:	/* size spec */
    switch(t2){
    case DT_UNSIGNED: t = DT_UNSIGNED_SHORT; goto ok;
    case DT_INT: t = DT_SHORT; goto ok;
    default: { break; }
    }
    break;
  }
err:
  error_at_node(x,"illegal type combination");
  t = DT_UNDEF;
ok:
  return(t);
}

/* debug */
void
dump_stubs(void)
{
  struct stub_gen_entry *ep;
  struct param_gen_desc *dp;
  int i,j,k;
  
  fprintf(stderr, "Module: %s , #entries : %d\n",current_module, n_stubs);
  for (i = 0; i < n_stubs; i++){
    ep = &stubs[i];
    fprintf(stderr, "\n*** entry name = %s\n",SYM_NAME(ep->ent_id));
    for(j = 0; j < ep->nparam; j++){
      dp = &ep->params[j];
      fprintf(stderr, "param(%d)= %s %s:%s",j,
	      basic_type_name(dp->param_type),SYM_NAME(dp->param_id),
	      mode_spec_name(dp->param_inout));
      fprintf(stderr, "\n");
      for(k = 0; k < dp->ndim; k++){
	  fprintf(stderr, "\t%d:",k);
	  expr_print(dp->dim[k].size_expr,stderr);
	  if(dp->dim[k].range_exprs){
	      fprintf(stderr, "\t\t+");
	      expr_print(dp->dim[k].range_exprs,stderr);
	  }
      }
    }
    if(ep->body != NULL) fprintf(stderr, "body = %s\n",ep->body);
    if(ep->description != NULL) 
      fprintf(stderr, "description = %s\n",ep->description);
    if(ep->required != NULL)
      fprintf(stderr, "required = %s\n",ep->required);
  }
}
