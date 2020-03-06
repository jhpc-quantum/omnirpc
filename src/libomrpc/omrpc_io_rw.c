static char rcsid[] = "$Id: omrpc_io_rw.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $";
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
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "omrpc_defs.h"
#include "omrpc_io.h"

void omrpc_io_recv(omrpc_io_handle_t *p, char *buf, int nbyte);
void omrpc_io_send(omrpc_io_handle_t *p, char *buf, int nbyte);

/*
 * read/write low level
 */
void omrpc_io_recv(omrpc_io_handle_t *p, char *buf, int nbyte)
{
    int n;
    char *cp,*cq;

    cp = buf;
    while(nbyte > 0){
        /* read from buffer */
        if(p->ncount <= p->pos) {
            omrpc_io_handle_fill(p);
            if(p->ncount == 0) {
                omrpc_fatal("omrpc_io_recv: unexpected EOF");
            }
        }
        cq = p->buf+p->pos;
        n = p->ncount - p->pos;
        if(n > nbyte){
            n = nbyte;
        }
        bcopy(cq,cp,n);  /* copy from buffer */
        p->pos += n;
        nbyte -= n;
        cp += n;
    }
}

void omrpc_io_send(omrpc_io_handle_t *p, char *buf, int nbyte)
{
    int n;
    char *cp,*cq;

    cp = buf;
    while(nbyte > 0){
        cq = p->buf+p->pos;
        n = OMRPC_IO_BUF_SIZE - p->pos;
        if(n > nbyte){
            n = nbyte;
        }
        bcopy(cp,cq,n);       /* copy into buffer */
        p->pos += n;
        nbyte -= n;
        cp += n;
        if(p->pos >= OMRPC_IO_BUF_SIZE){
            omrpc_io_handle_flush(p);
        }
    }
}


void omrpc_send_cmd(omrpc_io_handle_t *p, char cmd)
{
    omrpc_io_send(p,&cmd,1);
}

/* retrun cmd, and check EOF */
char omrpc_recv_cmd(omrpc_io_handle_t *p)
{
    char cmd;
    if(p->pos != 0 || p->ncount != 0)
        omrpc_fatal("omrpc_recv_cmd: not send_done");
    /* omrpc_io_recv(p,&cmd,1); */
    omrpc_io_handle_fill(p);
    if(p->ncount == 0){
        return EOF;
    } else {
        cmd = *(p->buf + p->pos++);
    }
    return cmd;
}

/*
 * low-level data transter
 */
void omrpc_send_byte(omrpc_io_handle_t *p,char *dp,int count)
{
    omrpc_io_send(p,dp,count);
}

void omrpc_recv_byte(omrpc_io_handle_t *p,char *dp,int count)
{
    omrpc_io_recv(p,dp,count);
}

/*
 * data type transfer function
 */

/* char */
void omrpc_send_char(omrpc_io_handle_t *p,char *dp,int count)
{
    omrpc_io_send(p,dp,sizeof(char)*count);
}

void omrpc_recv_char(omrpc_io_handle_t *p,char *dp,int count)
{
    omrpc_io_recv(p,dp,sizeof(char)*count);
}

void omrpc_send_recv_char(omrpc_io_handle_t *p, char *dp,int count,
                          int stride, int rw)
{
    if(stride == 1 || stride == 0){
        if(rw) omrpc_recv_char(p,dp,count);
        else omrpc_send_char(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw){
                omrpc_recv_char(p,dp,1);
            } else{
                omrpc_send_char(p,dp,1);
            }
            dp += stride;
        }
    }
}

/* u_char */
void omrpc_send_u_char(omrpc_io_handle_t *p,u_char *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(u_char)*count);
}

void omrpc_recv_u_char(omrpc_io_handle_t *p,u_char *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(u_char)*count);
}

void omrpc_send_recv_u_char(omrpc_io_handle_t *p, u_char *dp,int count,
                            int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_u_char(p,dp,count);
        else omrpc_send_u_char(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_u_char(p,dp,1);
            else omrpc_send_u_char(p,dp,1);
            dp += stride;
        }
    }
}

/* short */
void omrpc_send_short(omrpc_io_handle_t *p,short *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(short)*count);
}

void omrpc_recv_short(omrpc_io_handle_t *p,short *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(short)*count);
    if(p->swap_flag) omrpc_swap_short(dp,count);
}

void omrpc_send_recv_short(omrpc_io_handle_t *p, short *dp,int count,
                           int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_short(p,dp,count);
        else omrpc_send_short(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_short(p,dp,1);
            else omrpc_send_short(p,dp,1);
            dp += stride;
        }
    }
}

/* u_short */
void omrpc_send_u_short(omrpc_io_handle_t *p,u_short *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(u_short)*count);
}

void omrpc_recv_u_short(omrpc_io_handle_t *p,u_short *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(u_short)*count);
    if(p->swap_flag) omrpc_swap_short((short *)dp,count);
}


void omrpc_send_recv_u_short(omrpc_io_handle_t *p, u_short *dp,int count,
                             int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_u_short(p,dp,count);
        else omrpc_send_u_short(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_u_short(p,dp,1);
            else omrpc_send_u_short(p,dp,1);
            dp += stride;
        }
    }
}

/* int */
void omrpc_send_int(omrpc_io_handle_t *p,int *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(int)*count);
}

void omrpc_recv_int(omrpc_io_handle_t *p,int *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(int)*count);
    if(p->swap_flag) omrpc_swap_int(dp,count);
}


void omrpc_send_recv_int(omrpc_io_handle_t *p,int *dp,int count,
                         int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_int(p,dp,count);
        else omrpc_send_int(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_int(p,dp,1);
            else omrpc_send_int(p,dp,1);
            dp += stride;
        }
    }
}

/* u_int */
void omrpc_send_u_int(omrpc_io_handle_t *p,u_int *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(u_int)*count);
}

void omrpc_recv_u_int(omrpc_io_handle_t *p,u_int *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(u_int)*count);
    if(p->swap_flag) omrpc_swap_int((int*)dp,count);
}

void omrpc_send_recv_u_int(omrpc_io_handle_t *p, u_int *dp,int count,
                           int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_u_int(p,dp,count);
        else omrpc_send_u_int(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_u_int(p,dp,1);
            else omrpc_send_u_int(p,dp,1);
            dp += stride;
        }
    }
}

/* long */
void omrpc_send_long(omrpc_io_handle_t *p,long *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(long)*count);
}

void omrpc_recv_long(omrpc_io_handle_t *p,long *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(long)*count);
    if(p->swap_flag) omrpc_swap_int((int *)dp,count);
}

void omrpc_send_recv_long(omrpc_io_handle_t *p, long *dp,int count,
                          int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_long(p,dp,count);
        else omrpc_send_long(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_long(p,dp,1);
            else omrpc_send_long(p,dp,1);
            dp += stride;
        }
    }
}

/* u_long */
void omrpc_send_u_long(omrpc_io_handle_t *p,u_long *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(u_long)*count);
}

void omrpc_recv_u_long(omrpc_io_handle_t *p,u_long *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(long)*count);
    if(p->swap_flag) omrpc_swap_int((int *)dp,count);
}


void omrpc_send_recv_u_long(omrpc_io_handle_t *p, u_long *dp,int count,
                            int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_u_long(p,dp,count);
        else omrpc_send_u_long(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_u_long(p,dp,1);
            else omrpc_send_u_long(p,dp,1);
            dp += stride;
        }
    }
}

/* long_long */
void omrpc_send_long_long(omrpc_io_handle_t *p,long long *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(long long)*count);
}

void omrpc_recv_long_long(omrpc_io_handle_t *p,long long *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(long long)*count);
    if(p->swap_flag) omrpc_swap_long_long(dp,count);
}

void omrpc_send_recv_long_long(omrpc_io_handle_t *p, long long *dp,int count,
                               int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_long_long(p,dp,count);
        else omrpc_send_long_long(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_long_long(p,dp,1);
            else omrpc_send_long_long(p,dp,1);
            dp += stride;
        }
    }
}

/* u_long_long */
void omrpc_send_u_long_long(omrpc_io_handle_t *p,unsigned long long *dp,int count)
{

    omrpc_io_send(p,(char *)dp,sizeof(unsigned long long)*count);
}

void omrpc_recv_u_long_long(omrpc_io_handle_t *p,unsigned long long *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(unsigned long long)*count);
    if(p->swap_flag) omrpc_swap_long_long((long long*)dp,count);
}


void omrpc_send_recv_u_long_long(omrpc_io_handle_t *p, unsigned long long *dp,
                                 int count,int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_u_long_long(p,dp,count);
        else omrpc_send_u_long_long(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_u_long_long(p,dp,1);
            else omrpc_send_u_long_long(p,dp,1);
            dp += stride;
        }
    }
}

/* float */
void omrpc_send_float(omrpc_io_handle_t *p,float *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(float)*count);
}

void omrpc_recv_float(omrpc_io_handle_t *p,float *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(float)*count);
    if(p->swap_flag) omrpc_swap_float(dp,count);
}

void omrpc_send_recv_float(omrpc_io_handle_t *p, float *dp,int count,
                           int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_float(p,dp,count);
        else omrpc_send_float(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_float(p,dp,1);
            else omrpc_send_float(p,dp,1);
            dp += stride;
        }
    }
}

/* double */
void omrpc_send_double(omrpc_io_handle_t *p,double *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(double)*count);
}

void omrpc_recv_double(omrpc_io_handle_t *p,double *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(double)*count);
    if(p->swap_flag) omrpc_swap_double(dp,count);
}

void omrpc_send_recv_double(omrpc_io_handle_t *p, double *dp,int count,
                            int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_double(p,dp,count);
        else omrpc_send_double(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_double(p,dp,1);
            else omrpc_send_double(p,dp,1);
            dp += stride;
        }
    }
}

/* long double */
void omrpc_send_long_double(omrpc_io_handle_t *p,long double *dp,int count)
{
    omrpc_io_send(p,(char *)dp,sizeof(long double)*count);
}

void omrpc_recv_long_double(omrpc_io_handle_t *p,long double *dp,int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(long double)*count);
    if(p->swap_flag) omrpc_swap_long_double(dp,count);
}

void omrpc_send_recv_long_double(omrpc_io_handle_t *p, long double *dp,int count,
                                 int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_long_double(p,dp,count);
        else omrpc_send_long_double(p,dp,count);
    } else {
        while(count-- > 0){
            if(rw) omrpc_recv_long_double(p,dp,1);
            else omrpc_send_long_double(p,dp,1);
            dp += stride;
        }
    }
}

/* float complex */
void omrpc_send_float_complex(omrpc_io_handle_t *p, float _Complex *dp, int count)
{
    omrpc_io_send(p,(char*)dp,sizeof(float _Complex)*count);
}

void omrpc_recv_float_complex(omrpc_io_handle_t *p, float _Complex *dp, int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(float _Complex)*count);
    if(p->swap_flag) omrpc_swap_float_complex(dp,count);
}

void omrpc_send_recv_float_complex(omrpc_io_handle_t *p, float _Complex *dp, int count,
                                   int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_float_complex(p,dp,count);
        else omrpc_send_float_complex(p,dp,count);
    } else {
        while(count -- > 0){
            if(rw) omrpc_recv_float_complex(p,dp,count);
            else omrpc_send_float_complex(p,dp,count);
            dp += stride;
        }
    }
}

/* double complex */
void omrpc_send_double_complex(omrpc_io_handle_t *p, double _Complex *dp, int count)
{
    omrpc_io_send(p,(char*)dp,sizeof(double _Complex)*count);
}

void omrpc_recv_double_complex(omrpc_io_handle_t *p, double _Complex *dp, int count)
{
    omrpc_io_recv(p,(char *)dp,sizeof(double _Complex)*count);
    if(p->swap_flag) omrpc_swap_double_complex(dp,count);
}

void omrpc_send_recv_double_complex(omrpc_io_handle_t *p, double _Complex *dp, int count,
                                    int stride, int rw)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpc_recv_double_complex(p,dp,count);
        else omrpc_send_double_complex(p,dp,count);
    } else {
        while(count -- > 0){
            if(rw) omrpc_recv_double_complex(p,dp,count);
            else omrpc_send_double_complex(p,dp,count);
            dp += stride;
        }
    }
}

/* string */
void omrpc_send_recv_string(omrpc_io_handle_t *p, char **dp,int count,
                            int stride, int rw)
{
    int i;
    if(stride <= 1){
        if(rw){ /* recv */
            for(i = 0; i < count; i++) omrpc_recv_strdup(p, dp+i);
        } else {
            for(i = 0; i < count; i++) omrpc_send_string(p, dp[i]);
        }
    } else
        omrpc_fatal("no stride (%d) is supported in string send/recv",
                    stride);
}

void omrpc_send_string(omrpc_io_handle_t *p,char *cp)
{
    int len;
    len = 0;
    if(cp != NULL) len = strlen(cp)+1;
    omrpc_send_int(p,&len,1);
    omrpc_send_char(p,cp,len);
}

void omrpc_recv_string(omrpc_io_handle_t *p,char *cp)
{
    int len;
    omrpc_recv_int(p,&len,1);
    omrpc_recv_char(p,cp,len);
}

void omrpc_recv_strdup(omrpc_io_handle_t *p,char **cpp)
{
    int len;
    char *cp;
    omrpc_recv_int(p,&len,1);
    if(len != 0) {
        cp = (char *)omrpc_malloc(len+1);
        omrpc_recv_char(p,cp,len);
        *cpp = cp;
    } else
        *cpp = NULL;
}

/* file discripter */
void omrpc_send_file(omrpc_io_handle_t *hp,int fd)
{
    off_t off;
    int len,nbytes,r;

    /* check size */
    off = lseek(fd,0,SEEK_END);
    if(off < 0) omrpc_fatal("omrpc_send_file: lseek failed");
    if(off > INT_MAX) omrpc_fatal("omrpc_send_file: too large file");
    len = off;
    lseek(fd,0,SEEK_SET);

    omrpc_send_int(hp,&len,1);
    omrpc_io_handle_flush(hp);	/* flush once */
    while(len > 0){
        nbytes = len;
        if(nbytes > OMRPC_IO_BUF_SIZE) nbytes = OMRPC_IO_BUF_SIZE;
        r = read(fd,hp->buf,nbytes);
        if(nbytes != r) omrpc_fatal("omrpc_send_file: read fail r=%d",r);
        hp->pos = nbytes;
        len -= nbytes;
        omrpc_io_handle_flush(hp);
    }
}

void omrpc_recv_file(omrpc_io_handle_t *hp,int fd)
{
    int len, nbytes,r;

    omrpc_recv_int(hp,&len,1);

    /* write out current buffer */
    nbytes = hp->ncount - hp->pos;
    if(nbytes > len) nbytes = len;
    if(nbytes > 0){
        r = write(fd,hp->buf+hp->pos,nbytes);
        if(r != nbytes) omrpc_fatal("omrpc_recv_file: write fail r=%d",r);
        len -= nbytes;
    }
    while(len > 0){
        omrpc_io_handle_fill(hp);
        if(hp->ncount == 0){
            omrpc_fatal("omrpc_recv_file: unexpected EOF");
        }
        nbytes = len;
        if(nbytes > hp->ncount) nbytes = hp->ncount;
        r = write(fd,hp->buf,nbytes);
        if(r != nbytes) omrpc_fatal("omrpc_recv_file: write fail r=%d",r);
        len -= nbytes;
        hp->pos = nbytes;
    }
}

/*
 * swap routine
 */
void omrpc_swap_short(short *sp, int n)
{
    int i;
    char *p,c;

    for(i = 0; i < n; i++){
        p = (char *) sp++;
        c = p[0];
        p[0] = p[1];
        p[1] = c;
    }
}

void omrpc_swap_int(int *ip, int n)
{
    int i;
    char *p,c;

    for(i = 0; i < n; i++){
        p = (char *) ip++;
        c = p[0];
        p[0] = p[3];
        p[3] = c;

        c = p[1];
        p[1] = p[2];
        p[2] = c;
    }
}

void omrpc_swap_float(float *fp, int n)
{
    int i;
    char *p,c;

    for(i = 0; i < n; i++){
        p = (char *) fp++;

        c = p[0];
        p[0] = p[3];
        p[3] = c;

        c = p[1];
        p[1] = p[2];
        p[2] = c;
    }
}

void omrpc_swap_double(double *dp, int n)
{
    int i;
    char *p,c;

    for(i = 0; i < n; i++){
        p = (char *) dp++;
        c = p[0];
        p[0] = p[7];
        p[7] = c;

        c = p[1];
        p[1] = p[6];
        p[6] = c;

        c = p[2];
        p[2] = p[5];
        p[5] = c;

        c = p[3];
        p[3] = p[4];
        p[4] = c;
    }
}

void omrpc_swap_long_double(long double *ld, int n)
{
    int i;
    char *p,c;
    for(i = 0; i < n; i++){
        p = (char *) ld++;
        c = p[0];
        p[0] = p[11];
        p[11] = c;

        c = p[1];
        p[1] = p[10];
        p[10] = c;

        c = p[2];
        p[2] = p[9];
        p[9] = c;

        c = p[3];
        p[3] = p[8];
        p[8] = c;

        c = p[4];
        p[4] = p[7];
        p[7] = c;

        c = p[5];
        p[5] = p[6];
        p[6] = c;
    }
}


void omrpc_swap_long_long(long long *lp, int n)
{
    int i;
    char *p,c;

    for(i = 0; i < n; i++){
        p = (char *) lp++;
        c = p[0];
        p[0] = p[7];
        p[7] = c;

        c = p[1];
        p[1] = p[6];
        p[6] = c;

        c = p[2];
        p[2] = p[5];
        p[5] = c;

        c = p[3];
        p[3] = p[4];
        p[4] = c;
    }
}

void omrpc_swap_float_complex(float _Complex *fcp, int n)
{
    int i;
    float *p;

    for(i = 0; i < n; i++){
        p = (float *) fcp++;
        omrpc_swap_float(&(p[0]),1);
        omrpc_swap_float(&(p[1]),1);
    }
}

void omrpc_swap_double_complex(double _Complex *dcp, int n)
{
    int i;
    double *p;

    for(i = 0; i < n; i++){
        p = (double *) dcp++;
        omrpc_swap_double(&(p[0]),1);
        omrpc_swap_double(&(p[1]),1);
    }
}
