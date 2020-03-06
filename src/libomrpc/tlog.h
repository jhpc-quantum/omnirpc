/* 
 * $Id: tlog.h,v 1.1.1.1 2004-11-03 21:01:19 yoshihiro Exp $
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
#ifndef _TLOG_H
#define _TLOG_H

#include "tlog-omrpc.h"

#define TLOG_BLOCK_SIZE 1024
typedef struct _TLOG_BLOCK {
    struct _TLOG_BLOCK *next;
    double data[TLOG_BLOCK_SIZE/sizeof(double)];
} TLOG_BLOCK;

/* every log record is 2 double words. */
typedef struct tlog_record {
    char log_type;	/* major type */
    char proc_id;	/* processor id */
    short arg1;		/* minor type */
    int arg2;
    double time_stamp;
} TLOG_DATA;

typedef struct tlog_handle {
    TLOG_BLOCK *block_top;
    TLOG_BLOCK *block_tail;
    TLOG_DATA *free_p;
    TLOG_DATA *end_p;
} TLOG_HANDLE;
 
extern TLOG_HANDLE tlog_handle_table[];

/* prototypes */
void tlog_init(char *name);
void tlog_finalize(void);
void tlog_log(int id,enum tlog_type type);
void tlog_log1(int id,TLOG_TYPE type,int arg1);
void tlog_log2(int id,TLOG_TYPE type,int arg1,int arg2);
double tlog_timestamp(void);

#endif /* _TLOG_H */












