/* 
 * $Id: ninf_comm_lib.h,v 1.2 2006-01-25 16:06:17 ynaka Exp $
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
#ifndef __NINF_COMM_LIB_H__
#define __NINF_COMM_LIB_H__

#include <stdio.h>
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "ninf_stub_info.h"

#define OMRPC_ACK_OK 0
#define OMRPC_ACK_NG 1

/* Ninf Protocol */
#define OMRPC_REQ_KILL           	1
#define OMRPC_REQ_STUB_INFO       	2
#define OMRPC_REQ_STUB_INFO_BY_INDEX   	3
#define OMRPC_REQ_CALL           	4
#define OMRPC_GET_DESCIPTION		5

#define OMRPC_REQ_STUB_INFO_LOCAL       10
#define OMRPC_REQ_STUB_INFO_LOCAL_BY_INDEX     11

#define OMRPC_STOP_HB                   20

typedef union _any_t_union {
    char c;
    unsigned char uc;
    short s;
    unsigned short us;

    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;

    int16_t i16;
    u_int16_t ui16;
    int32_t i32;
    u_int32_t ui32;
    int64_t i64;
    u_int64_t ui64;
    
    float f;
    double d;
    long double ld;

    void * p;
    off_t ot;
    size_t st;

    float _Complex fc;
    double _Complex dc;
} any_t;

typedef struct _ninf_array_shape_info {
    int32_t ndim;
    int32_t shrink;
    DATA_TYPE elem_type;	/* element type */
    int32_t total_count;
    struct {
	int32_t bsize;	/* size of sub-dimension in byte */
	int32_t start;
	int32_t end;
	int32_t step;
    }  shape[MAX_DIM];
} ninf_array_shape_info;

/* prototype */
void ninf_recv_scalar_args(omrpc_io_handle_t *hp, NINF_STUB_INFO *sp,
			   any_t *args, int is_client);
void ninf_send_scalar_args(omrpc_io_handle_t *hp, NINF_STUB_INFO * sp,
			   any_t *args,int is_client);
void ninf_recv_send_any(omrpc_io_handle_t *hp, DATA_TYPE dt,
			void *p, int count, int stride,int rw);
void ninf_set_array_shape(struct ninf_param_desc *dp, 
			  NINF_STUB_INFO *sp, any_t *args,
			  ninf_array_shape_info *ap);
void ninf_recv_array(omrpc_io_handle_t *hp,char *base, 
		     ninf_array_shape_info *ap);
void ninf_send_array(omrpc_io_handle_t *hp,char *base, 
		     ninf_array_shape_info *ap);

/* defined in ninf_misc.o */
NINF_STUB_INFO * ninf_new_stub_info(void);
struct ninf_param_desc * ninf_new_param_desc(int num);
int data_type_size(DATA_TYPE dt);

void ninf_print_stub_info(FILE *fp,NINF_STUB_INFO *ip);

/* omrpc_call.c */
void omrpc_send_stub_info(omrpc_io_handle_t *hp,NINF_STUB_INFO  *sp);
void omrpc_send_kill(omrpc_io_handle_t *hp);
int omrpc_setup_args(va_list *app, any_t *args,NINF_STUB_INFO  *sp);
int omrpc_call_send_args(omrpc_io_handle_t *hp,NINF_STUB_INFO  *sp, 
			 any_t *args);
int omrpc_call_recv_args(omrpc_io_handle_t *hp,NINF_STUB_INFO  *sp, 
			 any_t *args);

void ninf_free_stub_info(NINF_STUB_INFO *sp);

#endif /* !__NINF_COMM_LIB_H__ */
