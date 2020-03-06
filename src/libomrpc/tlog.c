static char rcsid[] = "$Id: tlog.c,v 1.1.1.1 2004-11-03 21:01:19 yoshihiro Exp $";
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
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "omrpc_defs.h"
#include "tlog.h"

TLOG_HANDLE tlog_handle_table[MAX_THREADS];

static TLOG_DATA *tlog_get_data(int id);
static void tlog_dump(void);
static void tlog_block_swap_bytes(TLOG_DATA *dp);
static double start_time;

static char *tlog_fname;

void tlog_sig_handler(int x)
{
    tlog_finalize();
    exit(0);
}

void tlog_init(char *file_name)
{
    omrpc_prf("log on ...\n");
    tlog_fname = file_name;
    start_time = tlog_timestamp();
    tlog_log(0,TLOG_START);
    signal(SIGINT, tlog_sig_handler);
}

void tlog_finalize()
{
    TLOG_HANDLE *hp;
    int i;

    /* fprintf(stderr,"finalize log by %d ...\n", omp_get_thread_num()); */
    omrpc_prf("finalize log ...\n");
    for(i = 0; i < MAX_THREADS; i++){
        hp = &tlog_handle_table[i];
        if(hp->block_top == NULL) continue; /* not used */
        tlog_log(i,TLOG_END);
    }
    tlog_dump();
}

static TLOG_DATA *tlog_get_data(int id)
{
    TLOG_DATA *dp;
    TLOG_HANDLE *hp;
    TLOG_BLOCK *bp;

    hp = &tlog_handle_table[id];
    if((dp = hp->free_p) == NULL || dp >= hp->end_p){
        bp = (TLOG_BLOCK *)malloc(sizeof(TLOG_BLOCK));
        bzero(bp,sizeof(*bp));
        bp->next = NULL;
        if(hp->block_top == NULL){
            hp->block_top = hp->block_tail = bp;
        } else {
            hp->block_tail->next = bp;
            hp->block_tail = bp;
        }
        hp->free_p = (TLOG_DATA *)bp->data;
        hp->end_p = (TLOG_DATA *)((char *)bp->data + TLOG_BLOCK_SIZE);
        dp = hp->free_p;
    }
    hp->free_p = dp+1;
    dp->proc_id = id;
    dp->time_stamp = tlog_timestamp() - start_time;
    return dp;
}

void tlog_log(int id,enum tlog_type type)
{
    TLOG_DATA *dp;
    dp = tlog_get_data(id);
    dp->log_type = type;
}

void tlog_log1(int id,TLOG_TYPE type,int arg1)
{
    TLOG_DATA *dp;
    dp = tlog_get_data(id);
    dp->log_type = type;
    dp->arg1 = (short)arg1;
}

void tlog_log2(int id,TLOG_TYPE type,int arg1,int arg2)
{
    TLOG_DATA *dp;
    dp = tlog_get_data(id);
    dp->log_type = type;
    dp->arg1 = (short)arg1;
    dp->arg2 = (int)arg2;
}

static void tlog_dump()
{
    FILE *fp;
    int i;
    union {
        long i;
        char c;
    } x;
    int bigendian;
    TLOG_BLOCK *bp;

    x.i = 1;
    bigendian = (x.c == 0);

    if((fp = fopen(tlog_fname,"w")) == NULL){
        omrpc_prf("cannot open '%s'\n",tlog_fname);
        return;
    }

    {
        TLOG_BLOCK *bps[MAX_THREADS];
        int done;
        for(i = 0; i < MAX_THREADS; i++)
            bps[i] = tlog_handle_table[i].block_top;
        do {
            done = 1;
            for(i = 0; i < MAX_THREADS; i++){
                bp = bps[i];
                if(bp != NULL){
                    done = 0;
                    bps[i] = bp->next;
                    if(!bigendian) tlog_block_swap_bytes((TLOG_DATA *)bp->data);
                    if(fwrite((void *)bp->data,1,TLOG_BLOCK_SIZE,fp) 
                       != TLOG_BLOCK_SIZE){
                        omrpc_prf("write error to '%s'\n",tlog_fname);
                        return;
                    }
                }
            }
        } while(!done);
    }

    fclose(fp);
}

static void tlog_block_swap_bytes(TLOG_DATA *dp)
{
    union {
        char c[8];
        short int s;
        long int l;
        double d;
    } x;
    char t;
    TLOG_DATA *end_dp;

    end_dp = (TLOG_DATA *)(((char *)dp) + TLOG_BLOCK_SIZE);
    for(; dp < end_dp; dp++){
        x.s = dp->arg1;
        t = x.c[0]; x.c[0] = x.c[1]; x.c[1] = t;
        dp->arg1 = x.s;
        x.l = dp->arg2;
        t = x.c[0]; x.c[0] = x.c[3]; x.c[3] = t;
        t = x.c[1]; x.c[1] = x.c[2]; x.c[2] = t;
        dp->arg2 = x.l;
        x.d = dp->time_stamp;
        t = x.c[0]; x.c[0] = x.c[7]; x.c[7] = t;
        t = x.c[1]; x.c[1] = x.c[6]; x.c[6] = t;
        t = x.c[2]; x.c[2] = x.c[5]; x.c[5] = t;
        t = x.c[3]; x.c[3] = x.c[4]; x.c[4] = t;
        dp->time_stamp = x.d;
    }
}

