static char rcsid[] = "$Id: ninf_misc.c,v 1.2 2006-01-25 16:06:17 ynaka Exp $";
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
#include "omrpc_defs.h"
#include "ninf_comm_lib.h"

static char *data_type_names[] = DATA_TYPE_NAMES;
static char *mode_spec_names[] = MODE_SPEC_NAMES;
static char *value_type_names[] = VALUE_TYPE_NAMES;

int ninf_debug_flag = 0;

NINF_STUB_INFO * ninf_new_stub_info()
{
    NINF_STUB_INFO * tmp =
	(NINF_STUB_INFO *) omrpc_malloc(sizeof(NINF_STUB_INFO));
    tmp->params = NULL;
    tmp->description = NULL;
    return tmp;
}

struct ninf_param_desc * ninf_new_param_desc(int num)
{
    struct ninf_param_desc *  tmp = 
	(struct ninf_param_desc *) 
	omrpc_malloc(sizeof(struct ninf_param_desc) * num);
    return tmp;
}

void ninf_free_stub_info(NINF_STUB_INFO *sp)
{
    omrpc_free(sp->params);
    omrpc_free(sp);
}

int data_type_size(DATA_TYPE dt)
{
    switch(dt){
    case DT_CHAR:
	return sizeof(char);
    case DT_SHORT:
	return sizeof(short);
    case DT_INT:
	return sizeof(int);
    case DT_LONG:
	return sizeof(long);
    case DT_UNSIGNED_CHAR:
	return sizeof(unsigned char);
    case DT_UNSIGNED_SHORT:
	return sizeof(unsigned short);
    case DT_UNSIGNED:
	return sizeof(unsigned int);
    case DT_UNSIGNED_LONG:
	return sizeof(unsigned long);
    case DT_FLOAT:
	return sizeof(float);
    case DT_DOUBLE:
	return sizeof(double);
    case DT_UNSIGNED_LONGLONG:
	return sizeof(long long);
    case DT_LONGLONG:
	return sizeof(unsigned long long);
    case DT_STRING_TYPE:
	return sizeof(char *);
    case DT_LONG_DOUBLE:
	return sizeof(long double);
    case DT_SCOMPLEX:
	return sizeof(float _Complex);
    case DT_DCOMPLEX:
	return sizeof(double _Complex);
    case DT_FILENAME:
        return sizeof(char *);
    case DT_FILEPOINTER:
        return sizeof(FILE *);
    default:
	omrpc_fatal("data_type_size: bad data type");
    }
    return -1;
}

/* for debug, show interface */
void ninf_print_stub_info(FILE *fp,NINF_STUB_INFO *ip)
{
    struct ninf_param_desc *p;
    int i,k;

    fprintf(fp,"version: %d.%d\n",ip->version_major,ip->version_minor);
    fprintf(fp,"info_type: %d\n",ip->info_type);
    fprintf(fp,"module.name: %s.%s\n",ip->module_name,ip->entry_name);
    fprintf(fp,"nparam: %d\n",ip->nparam);
    for(i = 0; i < ip->nparam; i++){
	p = &(ip->params[i]);
	fprintf(fp,"  arg(%d): type=%s mode=%s ndim=%d\n",
		i,data_type_names[p->param_type],
		mode_spec_names[p->param_inout],p->ndim);
	for(k = 0; k < p->ndim; k++)
	    fprintf(fp,"    %d: size(%s),start(%s),end(%s),step(%s)\n",
		   k,value_type_names[p->dim[k].size_type],
		   value_type_names[p->dim[k].start_type],
		   value_type_names[p->dim[k].end_type],
		   value_type_names[p->dim[k].step_type]);
    }
    fprintf(fp,"order type: %s\n",value_type_names[ip->order_type]);
    fprintf(fp,"description:\n%s\n",
	    ip->description == NULL? "<null>":ip->description);
}

