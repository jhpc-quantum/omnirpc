static char rcsid[] = "$Id: ninf_comm_lib.c,v 1.2 2006-01-25 16:06:17 ynaka Exp $";
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
#include <stdio.h>
#include <limits.h>
#include "ninf_comm_lib.h"

/* static prototype */
static void init_EXP_STACK();
static void push_EXP_STACK(int item);
static int pop_EXP_STACK();
static int ninf_eval_expression(NINF_EXPRESSION * exp, NINF_STUB_INFO * sp, 
				any_t * args);
static int ninf_get_value(VALUE_TYPE value_type,  int value,
			  NINF_EXPRESSION * exp, NINF_STUB_INFO *sp,
			  any_t *args, int default_value);

/* 
 * send & recv scalar arguments
 */
void ninf_recv_scalar_args(omrpc_io_handle_t *hp, NINF_STUB_INFO *sp,
			   any_t *args, int is_client)
{
    int i;
    struct ninf_param_desc *dp;
  
    /* recv IN scalar variables */
    for(i = 0; i < sp->nparam; i++){
	dp = &sp->params[i];
	if(dp->param_type == DT_FUNC_TYPE) 
	    omrpc_fatal("call back func not supported");

	if(dp->ndim != 0) continue;
	if(!IS_IN_MODE(dp->param_inout)) break;

	switch(dp->param_type){
	case DT_CHAR:
	    omrpc_recv_char(hp,&args[i].c,1);
	    break;
	case DT_UNSIGNED_CHAR:
	    omrpc_recv_u_char(hp,&args[i].uc,1);
	    break;
	case DT_SHORT:
	    omrpc_recv_short(hp,&args[i].s,1);
	    break;
	case DT_UNSIGNED_SHORT:
	    omrpc_recv_u_short(hp,&args[i].us,1);
	    break;
	case DT_INT:
	    omrpc_recv_int(hp,&args[i].i,1);
	    break;
	case DT_UNSIGNED:
	    omrpc_recv_u_int(hp,&args[i].ui,1);
	    break;
	case DT_LONG:
	    omrpc_recv_long(hp,&args[i].l,1);
	    break;
	case DT_UNSIGNED_LONG:
	    omrpc_recv_u_long(hp,&args[i].ul,1);
	    break;
	case DT_FLOAT:
	    omrpc_recv_float(hp,&args[i].f,1);
	    break;
	case DT_DOUBLE:
	    omrpc_recv_double(hp,&args[i].d,1);
	    break;
	case DT_STRING_TYPE:
	    omrpc_recv_strdup(hp,(char **)(& args[i].p));
	    break;
	case DT_UNSIGNED_LONGLONG:
	    omrpc_recv_long_long(hp,&args[i].ll,1);
	    break;
	case DT_LONGLONG:
	    omrpc_recv_u_long_long(hp,&args[i].ull,1);
	    break;
	case DT_SCOMPLEX:
	    omrpc_recv_float_complex(hp,&args[i].fc,1);
	    break;
	case DT_DCOMPLEX:
	    omrpc_recv_double_complex(hp,&args[i].dc,1);
	    break;
	case DT_LONG_DOUBLE:
	    omrpc_recv_long_double(hp,&args[i].ld,1);
	    break;
        case DT_FILENAME:
            omrpc_recv_filename(hp, (char **)(& args[i].p));
            break;
        case DT_FILEPOINTER:
            omrpc_recv_filepointer(hp,(FILE **)(&args[i].p));
            break;
	default:
	    goto err;
	    break;
	}
    }
    return;
err:
    omrpc_fatal("ninf_recv_scalar_args: error");
}

void ninf_send_scalar_args(omrpc_io_handle_t *hp, NINF_STUB_INFO * sp,
			  any_t *args,int is_client)
{
    int i;
    struct ninf_param_desc *dp;
    
    /* recv IN scalar variables */
    for(i = 0; i < sp->nparam; i++){
	dp = &sp->params[i];
	if(dp->param_type == DT_FUNC_TYPE) 
	    omrpc_fatal("call back func not supported");

	if(dp->ndim != 0) continue;
	if(!IS_IN_MODE(dp->param_inout)) break;

	switch(dp->param_type){
	case DT_CHAR:
	    omrpc_send_char(hp,&args[i].c,1);
	    break;
	case DT_UNSIGNED_CHAR:
	    omrpc_send_u_char(hp,&args[i].uc,1);
	    break;
	case DT_SHORT:
	    omrpc_send_short(hp,&args[i].s,1);
	    break;
	case DT_UNSIGNED_SHORT:
	    omrpc_send_u_short(hp,&args[i].us,1);
	    break;
	case DT_INT:
	    omrpc_send_int(hp,&args[i].i,1);
	    break;
	case DT_UNSIGNED:
	    omrpc_send_u_int(hp,&args[i].ui,1);
	    break;
	case DT_LONG:
	    omrpc_send_long(hp,&args[i].l,1);
	    break;
	case DT_UNSIGNED_LONG:
	    omrpc_send_u_long(hp,&args[i].ul,1);
	    break;
	case DT_FLOAT:
	    omrpc_send_float(hp,&args[i].f,1);
	    break;
	case DT_DOUBLE:
	    omrpc_send_double(hp,&args[i].d,1);
	    break;
	case DT_STRING_TYPE:
	    omrpc_send_string(hp,args[i].p);
	    break;
	case DT_UNSIGNED_LONGLONG:
	    omrpc_send_long_long(hp,&args[i].ll,1);
	    break;
	case DT_LONGLONG:
	    omrpc_send_u_long_long(hp,&args[i].ull,1);
	    break;
	case DT_SCOMPLEX:
	    omrpc_send_float_complex(hp,&args[i].fc,1);
	    break;
	case DT_DCOMPLEX:
	    omrpc_send_double_complex(hp,&args[i].dc,1);
	    break;
	case DT_LONG_DOUBLE:
	    omrpc_send_long_double(hp,&args[i].ld,1);
	    break;
        case DT_FILENAME:
            omrpc_send_filename(hp, (char **)(&args[i].p));
            break;
        case DT_FILEPOINTER:
            omrpc_send_filepointer(hp, (FILE **)(&args[i].p));
            break;
	default:
	    goto err;
	    break;
	}
    }
    return;
err:
    omrpc_fatal("ninf_send_scalar_args: error");
}

/* 
 * send and recv any values with stride
 */
void ninf_recv_send_any(omrpc_io_handle_t *hp, DATA_TYPE dt,
		       void *p, int count, int stride,int rw)
{
    /* recv IN scalar variables */
    switch(dt){
    case DT_CHAR:
	omrpc_send_recv_char(hp,(char *)p,count,stride,rw);
	break;
    case DT_UNSIGNED_CHAR:
	omrpc_send_recv_u_char(hp,(unsigned char *)p,count,stride,rw);
	break;
    case DT_SHORT:
	omrpc_send_recv_short(hp,(short *)p,count,stride,rw);
	break;
    case DT_UNSIGNED_SHORT:
	omrpc_send_recv_u_short(hp,(unsigned short *)p,count,stride,rw);
	break;
    case DT_INT:
	omrpc_send_recv_int(hp,(int *)p,count,stride,rw);
	break;
    case DT_UNSIGNED:
	omrpc_send_recv_u_int(hp,(unsigned int *)p,count,stride,rw);
	break;
    case DT_LONG:
	omrpc_send_recv_long(hp,(long *)p,count,stride,rw);
	break;
    case DT_UNSIGNED_LONG:
	omrpc_send_recv_u_long(hp,(unsigned long *)p,count,stride,rw);
	break;
    case DT_FLOAT:
	omrpc_send_recv_float(hp,(float *)p,count,stride,rw);
	break;
    case DT_DOUBLE:
	omrpc_send_recv_double(hp,(double *)p,count,stride,rw);
	break;
    case DT_UNSIGNED_LONGLONG:
	omrpc_send_recv_u_long_long(hp,(unsigned long long *)p,
				    count,stride,rw);
	break;
    case DT_LONGLONG:
	omrpc_send_recv_long_long(hp,(long long *)p,
				     count,stride,rw);
	break;
    case DT_STRING_TYPE:
	omrpc_send_recv_string(hp,(char **)p,
			       count,stride,rw);
	break;
    case DT_SCOMPLEX:
	omrpc_send_recv_float_complex(hp,(float _Complex *)p,
				      count,stride,rw);
	break;
    case DT_DCOMPLEX:   /* should not be scalar */
	omrpc_send_recv_double_complex(hp,(double _Complex *)p,
				      count,stride,rw);
	break;
    case DT_LONG_DOUBLE:
	omrpc_send_recv_long_double(hp,(long double *)p,
				    count, stride, rw);
	break;
    case DT_FILENAME:
	omrpc_send_recv_filename(hp,(char **)p,count,stride,rw);
        break;
    case DT_FILEPOINTER:
	omrpc_send_recv_filepointer(hp,(FILE **)p,
                                    count,stride,rw);
	break;

    default:
	goto err;
	break;
    }
    return;
err:
    omrpc_fatal("ninf_recv_send_any: error");
}

/* fix array shape in array */
void ninf_set_array_shape(struct ninf_param_desc *dp, 
			  NINF_STUB_INFO *sp, any_t *args,
			  ninf_array_shape_info *ap)
{
    int bsize,size,start,end,step,total_count;
    int count = -INT_MAX;
    int d,ndim;

    ndim = dp->ndim;
    ap->ndim = ndim;
    ap->elem_type = dp->param_type;
    ap->shrink = sp->shrink;
    total_count = 1;
    bsize = data_type_size(dp->param_type);

    for (d = 0; d < ndim; d++){
	size = ninf_get_value(dp->dim[d].size_type,
			      dp->dim[d].size,
			      &(dp->dim[d].size_exp), sp,args,0);
	bsize *= size;
	ap->shape[d].bsize = bsize;
	
	start = ninf_get_value(dp->dim[d].start_type,
			       dp->dim[d].start,
			       &(dp->dim[d].start_exp), sp,args,0);
	ap->shape[d].start = start;
      
	end = ninf_get_value(dp->dim[d].end_type,
			     dp->dim[d].end,
			     &(dp->dim[d].end_exp), sp,args,size);
	ap->shape[d].end = end;
      
	step = ninf_get_value(dp->dim[d].step_type,
			      dp->dim[d].step,
			      &(dp->dim[d].step_exp), sp,args,1);
	ap->shape[d].step = step;
      
	/* calculuate the number of element to be sent */
	if (sp->shrink){
	    if(start < end && step > 0)
		count = (end - start + step-1)/step;	/* round */
	    else if((start > end) & (step < 0))
		count = (start - end + step-1)/step;
	    else omrpc_fatal("bad range value %d:%d,%d,%d",size,start,end,step);
	} else {
	    /* in no-shrink case, all the array shoud be transmitted */
	    count = size;
	}
	if(count != 0) total_count *= count;
    }
    ap->total_count = total_count;
}

/* 
 * send & recv array argument
 */
static void ninf_trans_array_rec(int dim, omrpc_io_handle_t *hp, char *base,
				ninf_array_shape_info *ap,int rw);

void ninf_recv_array(omrpc_io_handle_t *hp,char *base, 
		    ninf_array_shape_info *ap)
{
    if(ap->shrink){
	ninf_recv_send_any(hp, ap->elem_type,
			   base,ap->total_count,1,OMRPC_IO_R);
    }
    else {
	ninf_trans_array_rec(ap->ndim-1,hp,base,ap,OMRPC_IO_R);
    }
}

void ninf_send_array(omrpc_io_handle_t *hp,char *base, 
                     ninf_array_shape_info *ap)
{
    if(ap->shrink){
	ninf_recv_send_any(hp, ap->elem_type,
			   base,ap->total_count,1,OMRPC_IO_W);
    } else{
	ninf_trans_array_rec(ap->ndim-1,hp,base,ap,OMRPC_IO_W);
    }
}

static void ninf_trans_array_rec(int dim, omrpc_io_handle_t *hp, char *base,
				ninf_array_shape_info *ap,int rw)
{
  int i,bsize,start,end,step,count;

  start = ap->shape[dim].start;
  end = ap->shape[dim].end;
  step = ap->shape[dim].step;

  if (dim <= 0 ){
      count = (end-start+step-1)/step;
      ninf_recv_send_any(hp,ap->elem_type,base+start,count,step,rw);
  }else {
      bsize = ap->shape[dim-1].bsize;
      for(i = start; i < end; i += step){
	  ninf_trans_array_rec(dim-1,hp,base+i*bsize,ap,rw);
      }
  }
}

#if 0
/* 
 * send & recv interface information
 */
int trans_stub_info(dataTrans *DT, NINF_STUB_INFO *sp)
{
    int i,j, k;
    struct ninf_param_desc *dp;
    char *s;
    
    if(!trans_int(DT,&sp->version_major)) return(FALSE);
    if(!trans_int(DT,&sp->version_minor)) return(FALSE);
    if(!trans_int(DT,&sp->info_type)) return(FALSE);
    s = sp->module_name;
    if(!trans_string(DT,&s,NINF_MAX_NAME_LEN)) return(FALSE);
    s = sp->entry_name;
    if(!trans_string(DT,&s,NINF_MAX_NAME_LEN)) return(FALSE);
    if(!trans_int(DT,&sp->nparam)) return(FALSE);
    if (sp->params == NULL)
	sp->params = new_ninf_param_desc(sp->nparam);
    for(i = 0; i < sp->nparam; i++){
	dp = &sp->params[i];
	if(!trans_int(DT,(int *)&dp->param_type)) return(FALSE);
	if(!trans_int(DT,(int *)&dp->param_inout)) return(FALSE);
	if(!trans_int(DT,&dp->ndim)) return(FALSE);
	for(j = 0; j < abs(dp->ndim); j++){
	    if(!trans_enum(DT,(int *)&dp->dim[j].size_type)) return(FALSE);
	    if(!trans_int(DT,&dp->dim[j].size)) return(FALSE);
	    if (dp->dim[j].size_type == VALUE_BY_EXPR)
		for(k = 0; k < NINF_EXPRESSION_LENGTH; k++){
		    if(!trans_int(DT,(int *)&dp->dim[j].size_exp.type[k])) return(FALSE);
		    if(!trans_int(DT,&dp->dim[j].size_exp.val[k])) return(FALSE);
	}
	    if(!trans_enum(DT,(int *)&dp->dim[j].start_type)) return(FALSE);
	    if(!trans_int(DT,&dp->dim[j].start)) return(FALSE);
	    if (dp->dim[j].start_type == VALUE_BY_EXPR)
		for(k = 0; k < NINF_EXPRESSION_LENGTH; k++){
		    if(!trans_int(DT,(int *)&dp->dim[j].start_exp.type[k])) return(FALSE);
		    if(!trans_int(DT,&dp->dim[j].start_exp.val[k])) return(FALSE);
		}
	    if(!trans_enum(DT,(int *)&dp->dim[j].end_type)) return(FALSE);
	    if(!trans_int(DT,&dp->dim[j].end)) return(FALSE);
	    if (dp->dim[j].end_type == VALUE_BY_EXPR)
		for(k = 0; k < NINF_EXPRESSION_LENGTH; k++){
		    if(!trans_int(DT,(int *)&dp->dim[j].end_exp.type[k])) return(FALSE);
		    if(!trans_int(DT,&dp->dim[j].end_exp.val[k])) return(FALSE);
		}
	    if(!trans_enum(DT,(int *)&dp->dim[j].step_type)) return(FALSE);
	    if(!trans_int(DT,&dp->dim[j].step)) return(FALSE);
	    if (dp->dim[j].step_type == VALUE_BY_EXPR)
		for(k = 0; k < NINF_EXPRESSION_LENGTH; k++){
		    if(!trans_enum(DT,(int *)&dp->dim[j].step_exp.type[k])) return(FALSE);
		    if(!trans_int(DT,&dp->dim[j].step_exp.val[k])) return(FALSE);
		}
	}
    }
    if(!trans_enum(DT,(int *)&sp->order_type)) return(FALSE);
    if (sp->order_type == VALUE_BY_EXPR)
	for(k = 0; k < NINF_EXPRESSION_LENGTH; k++){
	    if(!trans_enum(DT,(int *)&sp->order.type[k])) return(FALSE);
	    if(!trans_int(DT,&sp->order.val[k])) return(FALSE);
	}
    if (send_description)
	if(!trans_string(DT,&(sp->description),NINF_MAX_NAME_LEN)) return(FALSE);
    if (!trans_int(DT,&(sp->shrink))) return(FALSE);
    return(TRUE);
}
#endif

/* 
 * evaluation routine for Ninf_Call
 */
static int EXP_STACK[NINF_EXPRESSION_LENGTH];  /* stack for eval expression */
static int EXP_STACK_POINTER = 0;

static void init_EXP_STACK()
{
    EXP_STACK_POINTER = 0;
}

static void push_EXP_STACK(int item)
{
    if (EXP_STACK_POINTER >= NINF_EXPRESSION_LENGTH)
	omrpc_fatal("Expression eval stack overflow");

    EXP_STACK[EXP_STACK_POINTER++] = item;
}

static int pop_EXP_STACK()
{
    if (EXP_STACK_POINTER <= 0)
	omrpc_fatal("Expression eval stack underflow");    

    return EXP_STACK[--EXP_STACK_POINTER];
}

static int ninf_eval_expression(NINF_EXPRESSION * exp, NINF_STUB_INFO * sp, 
				any_t * args)
{
    int i;
    init_EXP_STACK();
    for (i = 0; i < NINF_EXPRESSION_LENGTH; i++){
	VALUE_TYPE type = exp->type[i];
	int val =  exp->val[i];
	switch(type){
	case VALUE_NONE:
	case VALUE_CONST:
	case VALUE_IN_ARG:
	    push_EXP_STACK(ninf_get_value(type,val,NULL,sp,args,0));
	    break;
	case VALUE_OP:
	    switch(val){
	    case OP_VALUE_PLUS:
		push_EXP_STACK(pop_EXP_STACK() + pop_EXP_STACK());
		break;
	    case OP_VALUE_MINUS:
	    {
		int tmp = pop_EXP_STACK();
		push_EXP_STACK(pop_EXP_STACK() - tmp);
	    }
	    break;
	    case OP_VALUE_MUL:
		push_EXP_STACK(pop_EXP_STACK() * pop_EXP_STACK());
		break;
	    case OP_VALUE_DIV:
	    {
		int tmp = pop_EXP_STACK();
		push_EXP_STACK(pop_EXP_STACK() / tmp);
	    }
	    break;
	    case OP_VALUE_MOD:
	    {
		int tmp = pop_EXP_STACK();
		push_EXP_STACK(pop_EXP_STACK() % tmp);
	    }
	    break;
	    case OP_VALUE_UN_MINUS:
		push_EXP_STACK(- pop_EXP_STACK());
		break;
	    case OP_VALUE_EQ: {
		int right = pop_EXP_STACK();
		int left = pop_EXP_STACK();
		push_EXP_STACK(right == left);
		break;
	    }
	    case OP_VALUE_NEQ: {
		int right = pop_EXP_STACK();
		int left = pop_EXP_STACK();
		push_EXP_STACK(left != right);
		break;
	    }
	    case OP_VALUE_GT: {
		int right = pop_EXP_STACK();
		int left = pop_EXP_STACK();
		push_EXP_STACK(left > right);
		break;
	    }
	    case OP_VALUE_LT: {
		int right = pop_EXP_STACK();
		int left = pop_EXP_STACK();
		push_EXP_STACK(left < right);
		break;
	    }
	    case OP_VALUE_GE: {
		int right = pop_EXP_STACK();
		int left = pop_EXP_STACK();
		push_EXP_STACK(left >= right);
		break;
	    }
	    case OP_VALUE_LE: {
		int right = pop_EXP_STACK();
		int left = pop_EXP_STACK();
		push_EXP_STACK(left <= right);
		break;
	    }
	    case OP_VALUE_TRY: {
		int right = pop_EXP_STACK();
		int left = pop_EXP_STACK();
		int cond = pop_EXP_STACK();
		push_EXP_STACK(cond ? left : right);
		break;
	    }
	    default:
		omrpc_fatal("unknown OP_CODE");
	    }
	    break;
	case VALUE_END_OF_OP:
	    return pop_EXP_STACK();
	    /* break; */
	default:
	    omrpc_fatal("unknown VALUE_TYPE in eval expression");
	}
    }
    return pop_EXP_STACK();
}

static int ninf_get_value(VALUE_TYPE value_type,  int value,
			  NINF_EXPRESSION * exp, NINF_STUB_INFO *sp,
			  any_t *args, int default_value)
{
    int i;
    
    switch(value_type){
    case VALUE_NONE:
	return(default_value);
    case VALUE_CONST:
	return(value);
    case VALUE_IN_ARG:
	i = value;
	switch(sp->params[i].param_type){
	case DT_CHAR:
	    return(args[i].c);
	case DT_SHORT:
	    return(args[i].s);
	case DT_INT:
	    return(args[i].i);
	case DT_LONG:
	    return(args[i].l);
	case DT_UNSIGNED_CHAR:
	    return(args[i].uc);
	case DT_UNSIGNED_SHORT:
	    return(args[i].us);
	case DT_UNSIGNED:
	    return(args[i].ui);
	case DT_UNSIGNED_LONG:
	    return(args[i].ul);
	case DT_FLOAT:
	    return(args[i].f);
	case DT_DOUBLE:
	    return(args[i].d);
	case DT_LONG_DOUBLE:
	    return(args[i].ld);
	case DT_UNSIGNED_LONGLONG:
	    return(args[i].ull);
	case DT_LONGLONG:
	    return(args[i].ll);
	case DT_SCOMPLEX:            /* complex should not be scalar */
	    return(args[i].fc);
	case DT_DCOMPLEX:
	    return(args[i].dc);
        case DT_FILENAME:  /* it may be needless */
        case DT_FILEPOINTER: /* it may be needless */
	case DT_STRING_TYPE:
	default:
	    omrpc_fatal("unknown data type");
	    break;
	}
	break;
    case VALUE_BY_EXPR:
	if (exp == NULL)
	    omrpc_fatal("Expression in Expression");
	return ninf_eval_expression(exp,sp,args);
    default:
	omrpc_fatal("unknown VALUE_TYPE %d", value_type);
    }
    
    return -INT_MAX;
}
