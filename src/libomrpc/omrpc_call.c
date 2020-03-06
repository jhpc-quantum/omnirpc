static char rcsid[] = "$Id: omrpc_call.c,v 1.2 2006-01-25 16:06:17 ynaka Exp $";
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
#include <stdarg.h>
#include "omrpc_defs.h"
#include "ninf_stub_info.h"
#include "ninf_comm_lib.h"

int omrpc_fortran_flag;

void omrpc_send_kill(omrpc_io_handle_t *hp)
{
    if(omrpc_debug_flag) omrpc_prf("omrpc_send_kill .. \n");
    omrpc_send_cmd(hp,OMRPC_REQ_KILL);
    omrpc_send_done(hp);
}

/*
 * set up arguments
 */
int omrpc_setup_args(va_list *app, any_t *args,NINF_STUB_INFO  *sp)
{
    int i;
    char * tmp;
    struct ninf_param_desc *dp;

    for(i = 0; i < sp->nparam; i++){
        dp = &(sp->params[i]);
        if (dp->ndim == 0) {  /* scalar */
            switch(dp->param_type){
            case DT_CHAR:
                if (omrpc_fortran_flag)
                    args[i].c = (char)(*(va_arg(*app,int *)));
                else
                    args[i].c = (char)(va_arg(*app,int));
                break;
            case DT_SHORT:
                if (omrpc_fortran_flag)
                    args[i].s = (short)(*(va_arg(*app,int *)));
                else
                    args[i].s = (short)(va_arg(*app,int));
                break;
            case DT_INT:
                if (omrpc_fortran_flag)
                    args[i].i = *(va_arg(*app,int *));
                else
                    args[i].i = va_arg(*app,int);
                break;
            case DT_LONG:
                if (omrpc_fortran_flag)
                    args[i].l = *(va_arg(*app,long *));
                else
                    args[i].l = va_arg(*app,long);
                break;
            case DT_UNSIGNED_CHAR:
                if (omrpc_fortran_flag)
                    args[i].uc = (unsigned char)(*(va_arg(*app, unsigned int *)));
                else
                    args[i].uc = (unsigned char)(va_arg(*app,unsigned int));
                break;
            case DT_UNSIGNED_SHORT:
                if (omrpc_fortran_flag)
                    args[i].us = (unsigned short)(*(va_arg(*app, unsigned int *)));
                else
                    args[i].us = (unsigned short)(va_arg(*app,unsigned int));
                break;
            case DT_UNSIGNED:
                if (omrpc_fortran_flag)
                    args[i].ui = (unsigned int)(*(va_arg(*app, unsigned int *)));
                else
                    args[i].ui = va_arg(*app,unsigned int);
                break;
            case DT_UNSIGNED_LONG:
                if (omrpc_fortran_flag)
                    args[i].ul = (unsigned long)(*(va_arg(*app, unsigned long *)));
                else
                    args[i].ul = (unsigned long)va_arg(*app,unsigned long);
                break;
            case DT_FLOAT:
                if (omrpc_fortran_flag)
                    args[i].f = (float)(*(va_arg(*app, double *)));
                else
                    args[i].f = (float)(va_arg(*app,double));
                break;
            case DT_DOUBLE:
                if (omrpc_fortran_flag)
                    args[i].d = *(va_arg(*app, double *));
                else
                    args[i].d = va_arg(*app,double);
                break;
            case DT_STRING_TYPE:
                args[i].p = va_arg(*app, char *);
                break;
            case DT_LONG_DOUBLE:
                if (omrpc_fortran_flag)
                    args[i].ld = (long double)(*(va_arg(*app, long double *)));
                else
                    args[i].ld = (long double)va_arg(*app, long double);
                break;
            case DT_UNSIGNED_LONGLONG:
                if (omrpc_fortran_flag)
                    args[i].ull = (unsigned long long)(*(va_arg(*app, unsigned long long *)));
                else
                    args[i].ull = (unsigned long long)va_arg(*app, unsigned long long);
                break;
            case DT_LONGLONG:
                if (omrpc_fortran_flag)
                    args[i].ll = (long long)(*(va_arg(*app, long long *)));
                else
                    args[i].ll = (long long)va_arg(*app, long long);
                break;
            case DT_SCOMPLEX:
                if (omrpc_fortran_flag)
                    args[i].fc = (float _Complex)(*(va_arg(*app, float _Complex *)));
                else
                    args[i].fc = (float _Complex)va_arg(*app, float _Complex);
                break;
            case DT_DCOMPLEX:
                if (omrpc_fortran_flag)
                    args[i].dc = (double _Complex)(*(va_arg(*app, double _Complex *)));
                else
                    args[i].dc = (double _Complex)va_arg(*app, double _Complex);
                break;
            case DT_FILENAME: 
                {
                    tmp = va_arg(*app, char *);
                    if (IS_OUT_MODE(dp->param_inout))
                        args[i].p = strdup(tmp);
                    else
                        args[i].p = tmp;
                }
                break;
            case DT_FILEPOINTER:
                args[i].p = ((va_arg(*app, FILE *)));
                break;
            default:
                omrpc_fatal("unknown data type");
                break;
            }
        } else {
            args[i].p = (void *)va_arg(*app, void *);
	    /* omrpc_prf("Pointer set to %p\n",args[i].p);*/
            /* args[i].p = va_arg(*app, char *); */
        }
    }
    return TRUE;
}

/*
 * send args to start rpc
 */
int omrpc_call_send_args(omrpc_io_handle_t *hp, NINF_STUB_INFO  *sp, any_t *args)
{
    int i;
    ninf_array_shape_info array_shape;
    struct ninf_param_desc *dp;

    omrpc_send_cmd(hp,OMRPC_REQ_CALL);
    omrpc_send_short(hp,&sp->index,1);
    ninf_send_scalar_args(hp,sp,args,TRUE);

    /* receive vector data */
    for (i = 0; i < sp->nparam; i++){
        dp = &sp->params[i];
        if(dp->ndim == 0) continue; 	/* scalar */
        if(!IS_IN_MODE (dp->param_inout)) continue;
        /* compute the number of elemnts */
        ninf_set_array_shape(dp,sp,args,&array_shape);
        ninf_send_array(hp,args[i].p,&array_shape);
    }
    omrpc_send_done(hp);
    return TRUE;
}

/*
 * recv results from rpc
 */
int omrpc_call_recv_args(omrpc_io_handle_t *hp,NINF_STUB_INFO  *sp,
                         any_t *args)
{
    short ack;
    int i;
    ninf_array_shape_info array_shape;
    struct ninf_param_desc *dp;

    /* wait for return */
    ack = omrpc_recv_cmd(hp);	/* should check ? */
    /* ninf_recv_scalar_args(hp,sp,args,TRUE); */

    for (i = 0; i < sp->nparam; i++){
        dp = &sp->params[i];
        //test
        //        if(dp->param_type == DT_FILEPOINTER)
        //            (dp->ndim)++;
        //end
        if(dp->ndim == 0) continue; 	/* scalar */
        if(!IS_OUT_MODE (dp->param_inout)) continue;

        /* compute the number of elemnts */
        ninf_set_array_shape(dp,sp,args,&array_shape);
        ninf_recv_array(hp,args[i].p,&array_shape);
    }
    omrpc_recv_done(hp);

    return TRUE;
}



/*
 * stub info
 */
static void omrpc_recv_param_desc(omrpc_io_handle_t *hp,
                                  struct ninf_param_desc *dp);
static void omrpc_send_param_desc(omrpc_io_handle_t *hp,
                                  struct ninf_param_desc *dp);

int omrpc_req_stub_info(omrpc_io_handle_t *hp,char *name)
{
    omrpc_send_cmd(hp,OMRPC_REQ_STUB_INFO);
    omrpc_send_string(hp,name);
    omrpc_send_done(hp);
    if(omrpc_debug_flag) omrpc_prf("omrpc_req_stub_info ...\n");
    return TRUE;
}

int omrpc_req_stub_info_by_index(omrpc_io_handle_t *hp,short index)
{
    omrpc_send_cmd(hp,OMRPC_REQ_STUB_INFO_BY_INDEX);
    omrpc_send_short(hp,&index,1);
    omrpc_send_done(hp);
    if(omrpc_debug_flag) omrpc_prf("omrpc_req_stub_info ...\n");
    return TRUE;
}

int omrpc_recv_stub_info(omrpc_io_handle_t *hp,NINF_STUB_INFO  **spp)
{
    char ack;
    NINF_STUB_INFO *sp;
    struct ninf_param_desc *dp;
    int i;

    if(omrpc_debug_flag) omrpc_prf("omrpc_recv_stub_info ...\n");

    sp = ninf_new_stub_info();

    ack = omrpc_recv_cmd(hp);
    if(ack != OMRPC_ACK_OK){
        omrpc_recv_done(hp);
        return OMRPC_ERROR;
    }

    omrpc_recv_int(hp,&sp->version_major,1);
    omrpc_recv_int(hp,&sp->version_minor,1);
    omrpc_recv_int(hp,&sp->info_type,1);
    omrpc_recv_string(hp,sp->module_name);
    omrpc_recv_string(hp,sp->entry_name);
    omrpc_recv_int(hp,&sp->nparam,1);
    omrpc_recv_char(hp,&sp->shrink,1);
    omrpc_recv_char(hp,&sp->backend,1);
    omrpc_recv_short(hp,&sp->index,1);

    dp = ninf_new_param_desc(sp->nparam);
    sp->params = dp;
    for(i = 0; i < sp->nparam; i++){
        omrpc_recv_param_desc(hp,dp++);
    }
    sp->description = NULL;
    *spp = sp;

    if(omrpc_debug_flag){
        omrpc_prf("get_stub_info:");
        ninf_print_stub_info(stderr,sp);
    }
    omrpc_recv_done(hp);

    return OMRPC_OK;
}

static void omrpc_recv_param_desc(omrpc_io_handle_t *hp,
                                  struct ninf_param_desc *dp)
{
    int i;

    omrpc_recv_int(hp,(int *)&dp->param_type,1);
    omrpc_recv_int(hp,(int *)&dp->param_inout,1);
    omrpc_recv_int(hp,&dp->ndim,1);
    for(i = 0; i < dp->ndim; i++){
        omrpc_recv_int(hp,(int *)&dp->dim[i].size_type,1);
        omrpc_recv_int(hp,&dp->dim[i].size,1);
        omrpc_recv_int(hp,(int *)dp->dim[i].size_exp.type,NINF_EXPRESSION_LENGTH);
        omrpc_recv_int(hp,dp->dim[i].size_exp.val,NINF_EXPRESSION_LENGTH);

        omrpc_recv_int(hp,(int *)&dp->dim[i].start_type,1);
        omrpc_recv_int(hp,&dp->dim[i].start,1);
        omrpc_recv_int(hp,(int *)dp->dim[i].start_exp.type,NINF_EXPRESSION_LENGTH);
        omrpc_recv_int(hp,dp->dim[i].start_exp.val,NINF_EXPRESSION_LENGTH);

        omrpc_recv_int(hp,(int *)&dp->dim[i].end_type,1);
        omrpc_recv_int(hp,&dp->dim[i].end,1);
        omrpc_recv_int(hp,(int *)dp->dim[i].end_exp.type,NINF_EXPRESSION_LENGTH);
        omrpc_recv_int(hp,dp->dim[i].end_exp.val,NINF_EXPRESSION_LENGTH);

        omrpc_recv_int(hp,(int *)&dp->dim[i].step_type,1);
        omrpc_recv_int(hp,&dp->dim[i].step,1);
        omrpc_recv_int(hp,(int *)dp->dim[i].step_exp.type,NINF_EXPRESSION_LENGTH);
        omrpc_recv_int(hp,dp->dim[i].step_exp.val,NINF_EXPRESSION_LENGTH);
    }
}

void omrpc_send_stub_info(omrpc_io_handle_t *hp,NINF_STUB_INFO  *sp)
{
    struct ninf_param_desc *dp;
    int i;

    if(omrpc_debug_flag) omrpc_prf("omrpc_send_stub_info ...\n");

    omrpc_send_cmd(hp,OMRPC_ACK_OK);

    omrpc_send_int(hp,&sp->version_major,1);
    omrpc_send_int(hp,&sp->version_minor,1);
    omrpc_send_int(hp,&sp->info_type,1);
    omrpc_send_string(hp,sp->module_name);
    omrpc_send_string(hp,sp->entry_name);
    omrpc_send_int(hp,&sp->nparam,1);
    omrpc_send_char(hp,&sp->shrink,1);
    omrpc_send_char(hp,&sp->backend,1);
    omrpc_send_short(hp,&sp->index,1);

    dp = sp->params;
    for(i = 0; i < sp->nparam; i++){
        omrpc_send_param_desc(hp,dp++);
    }

    omrpc_send_done(hp);
}

static void omrpc_send_param_desc(omrpc_io_handle_t *hp,
                                  struct ninf_param_desc *dp)
{
    int i;

    omrpc_send_int(hp,(int *)&dp->param_type,1);
    omrpc_send_int(hp,(int *)&dp->param_inout,1);
    omrpc_send_int(hp,&dp->ndim,1);
    for(i = 0; i < dp->ndim; i++){
        omrpc_send_int(hp,(int *)&dp->dim[i].size_type,1);
        omrpc_send_int(hp,&dp->dim[i].size,1);
        omrpc_send_int(hp,(int *)dp->dim[i].size_exp.type,NINF_EXPRESSION_LENGTH);
        omrpc_send_int(hp,dp->dim[i].size_exp.val,NINF_EXPRESSION_LENGTH);

        omrpc_send_int(hp,(int *)&dp->dim[i].start_type,1);
        omrpc_send_int(hp,&dp->dim[i].start,1);
        omrpc_send_int(hp,(int *)dp->dim[i].start_exp.type,NINF_EXPRESSION_LENGTH);
        omrpc_send_int(hp,dp->dim[i].start_exp.val,NINF_EXPRESSION_LENGTH);

        omrpc_send_int(hp,(int *)&dp->dim[i].end_type,1);
        omrpc_send_int(hp,&dp->dim[i].end,1);
        omrpc_send_int(hp,(int *)dp->dim[i].end_exp.type,NINF_EXPRESSION_LENGTH);
        omrpc_send_int(hp,dp->dim[i].end_exp.val,NINF_EXPRESSION_LENGTH);

        omrpc_send_int(hp,(int *)&dp->dim[i].step_type,1);
        omrpc_send_int(hp,&dp->dim[i].step,1);
        omrpc_send_int(hp,(int *)dp->dim[i].step_exp.type,NINF_EXPRESSION_LENGTH);
        omrpc_send_int(hp,dp->dim[i].step_exp.val,NINF_EXPRESSION_LENGTH);
    }
}

