static char rcsid[] = "$Id: omrpc_io_rw_file.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $";
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
 * Time-stamp: <2005-08-30 23:50:38 ynaka>
 */

#include <limits.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ninf_comm_lib.h"

/* for file transfer */
#define TMP_FILE_TEMPLATE "omrpc_tmp.XXXXXX"
#define TMP_BUFFER_SIZE   1000
#define max(x,y)    ((x)>(y) ? (x) : (y))
#define min(x,y)    ((x)<(y) ? (x) : (y))

char *omrpc_working_path = WORKING_PATH_DEFAULT; /* default: "/tmp/" */
omrpc_io_fileio_name_t *omrpc_fileio_name_head = NULL;
omrpc_io_fileio_name_t *omrpc_fileio_name_tail;


static char *omrpc_make_tmp_file_name()
{
    char template[256];
    char filename[256];
    int fd;

    if(omrpc_working_path != NULL &&
       strlen(omrpc_working_path) != 0){
        snprintf(template, sizeof(template),
                "%s/%s", omrpc_working_path, TMP_FILE_TEMPLATE);
    } else {
        snprintf(template, sizeof(template),
                 "%s/%s", WORKING_PATH_DEFAULT, TMP_FILE_TEMPLATE);
    }

    strcpy(filename, template);
    fd = mkstemp(filename);
    if(fd < 0){
        omrpc_prf("omrpc_make_tmp_file_name: mkstemp\n");
        return NULL;
    } else {
        close(fd);
    }

    return strdup(filename);
}

//now working
static void omrpc_io_fileio_register_unlink(char *filename)
{
    if(filename == NULL){
        omrpc_prf("omrpc_io_fileio_register_unlink: null filename was specified.");
    }

    omrpc_io_fileio_name_t* p= omrpc_malloc(sizeof(omrpc_io_fileio_name_t));
    strncpy(p->name, filename, strlen(filename)+1);
    omrpc_fileio_name_head->next = p;
    p->prev = omrpc_fileio_name_head;

    omrpc_fileio_name_head = p;
}

static void omrpc_unlink_tmp_file_name(char *filename)
{
    if(unlink(filename) < 0){
        omrpc_prf("omrpc_unlink_tmp_file: unlink failed");
    }
}

static void omrpc_unlink_tmp_file_pointer(FILE *fp){
    /* dummy */
}

int omrpc_get_fp_size(FILE *fp)
{
    int current;
    int end;
    int size;

    if( fp == NULL){
        omrpc_fatal("illigal FILE pointer");
    }

    if((current = ftell(fp)) < 0){
        omrpc_error("omrpc_get_fp_size: ftell");
        return -1;
    }

    if(fseek(fp, 0, SEEK_END) < 0){
        omrpc_error("omrpc_get_fp_size: fseek end");
        return -1;
    }

    if((end = ftell(fp)) < 0){
        omrpc_error("omrpc_get_fp_size: ftell");
        return -1;
    }

    size = end - current;
    if(fseek(fp, current, SEEK_SET) < 0){
        omrpc_error("omrpc_get_fp_size: fseek set");
        return -1;
    }

    if(omrpc_debug_flag){
        omrpc_prf("omrpc_get_fp_size: current=%d, end=%d, size=%d\n",
                  current, end ,size);
    }

    return size;
}


FILE * omrpc_make_tmp_fp(char *mode)
{
    char *filename;
    FILE *fp;

    filename = omrpc_make_tmp_file_name();

    if((fp = fopen(filename, mode)) == NULL){
        omrpc_error("omrpc_make_tmp_fp: cannot open %s", filename);
        return NULL;
    }

    omrpc_io_fileio_register_unlink(filename);
    if(omrpc_debug_flag){
        omrpc_prf("omrpc_make_tmp_fp: filename=%s\n", filename);
    }

    return fp;
}

FILE * omrpc_make_tmp_fp_filename(char *mode, char **filenamep)
{
    char *filename;
    FILE *fp;

    filename = omrpc_make_tmp_file_name();
    if((fp = fopen(filename, mode)) == NULL){
        omrpc_error("omrpc_make_tmp_fp_filename: cannot create file\n");
        return NULL;
    }

    omrpc_io_fileio_register_unlink(filename);
    *filenamep = filename;

    return fp;
}


/* file pointer */
void omrpc_send_filepointer(omrpc_io_handle_t *hp, FILE **fpp)
{
    int len;
    int nbytes;
    int r;

    /* flush the file buffer */
    rewind(*fpp);
    fflush(*fpp);

    if (*fpp == NULL){
        len = -1;  /* inform the pointer was NULL */
        omrpc_send_int(hp, &len, 1);
        return;
    }

    if ((len = omrpc_get_fp_size(*fpp)) < 0){
        omrpc_error("omrpc_send_filepointer: cannot get file size\n");
    }

    /* send the size of file */
    omrpc_send_int(hp, &len, 1);
    omrpc_io_handle_flush(hp);  /* flush once */

    while(len > 0){
        nbytes = len;
        if(nbytes > OMRPC_IO_BUF_SIZE){
            nbytes = OMRPC_IO_BUF_SIZE;
        }
        r = fread(hp->buf,1, nbytes, *fpp);
        if(nbytes != r){
            omrpc_fatal("omrpc_send_file: read fail r=%d",r);
        }
        hp->pos = nbytes;
        len -= nbytes;
        omrpc_io_handle_flush(hp);
        if(omrpc_debug_flag){
            omrpc_prf("omrpc_send_filepointer: send flush: rest = %d\n", len);
        }
    }

    /* post proceeding */
    if(omrpc_is_client){
        ; /* nothing to do */
    } else {
        fclose(*fpp);
    }
}


void omrpc_recv_filepointer(omrpc_io_handle_t *hp, FILE **fpp)
{
    FILE *tmpfp;
    int len;
    int nbytes;
    int r;

    /* pre-procedure  */
    if(omrpc_is_client){
        ;
    } else {
        /* create new filepointer */
        tmpfp = omrpc_make_tmp_fp("w+");
        if (tmpfp == NULL){
            omrpc_error("omrpc_recv_filepointer: make_tmp_fp\n");
        }
        *fpp = (FILE *)tmpfp;
    }

    /* get file length */
    omrpc_recv_int(hp, &len, 1);
    if (omrpc_debug_flag){
        omrpc_prf("omrpc_recv_filepoiner: filesize = %d\n", len);
    }

    /* the filepointer / finename was null */
    if (len == -1){
        *fpp = NULL;
        return;
    }

    /* write out current buffer 
     * and send file to other side host
     */
    nbytes = hp->ncount - hp->pos;
    if(nbytes > len){
        nbytes = len;
    }
    if(nbytes > 0){
        r = fwrite(hp->buf + hp->pos, 1, nbytes, *fpp);
        if(r != nbytes){
            omrpc_fatal("omrpc_recv_file: write fail r=%d",r);
        }
        len -= nbytes;
        hp->pos += nbytes;
    }

    while(len > 0){
        omrpc_io_handle_fill(hp);
        if(hp->ncount == 0){
            omrpc_fatal("omrpc_recv_file: unexpected EOF");
        }
        nbytes = len;
        if(nbytes > hp->ncount){
            nbytes = hp->ncount;
        }
        r = fwrite(hp->buf, 1, nbytes, *fpp);
        if(r != nbytes){
            omrpc_fatal("omrpc_recv_file: write fail r=%d",r);
        }
        len -= nbytes;
        hp->pos = nbytes;
    }

    /* post proceeding */
    fflush(*fpp);
    rewind(*fpp);
}

void omrpc_send_recv_filepointer(omrpc_io_handle_t *hp, FILE **dp,
                                 int count, int stride, int rw)
{
    int i;

    if(stride <= 1){
        if(rw) { /* recv */
            for(i = 0; i < count; i++){
                omrpc_recv_filepointer(hp, &(dp[i]));
            }
        } else {
            for(i = 0; i < count; i++){
                omrpc_send_filepointer(hp, &(dp[i]));
            }
        }
    } else {
        omrpc_fatal("no stride (%d) is supported in filepointer send/recv",
                    stride);
    }
}

/* filename */
void omrpc_send_filename(omrpc_io_handle_t *hp, char **fname)
{
    int len;
    int nbytes;
    int r;
    FILE *tmpfp;

    /* pre-procedure */
    if(omrpc_is_client){
        tmpfp = fopen(*fname, "r");
        if(tmpfp == NULL){
            omrpc_error("omrpc_send_filepointer: cannot open: %s\n", *fname);
        }
    } else {
        tmpfp = fopen(*fname, "r");
        if(tmpfp == NULL){
            omrpc_error("omrpc_send_filepointer: cannot open: %s\n", *fname);
        }
    }

    if (tmpfp == NULL){
        len = -1;  /* inform the pointer was NULL */
        omrpc_send_int(hp, &len, 1);
        return;
    }

    if ((len = omrpc_get_fp_size(tmpfp)) < 0){
        omrpc_error("omrpc_send_filepointer: cannot get file size\n");
    }

    /* send file size */
    omrpc_send_int(hp, &len, 1);
    omrpc_io_handle_flush(hp);  /* flush once */

    while(len > 0){
        nbytes = len;
        if(nbytes > OMRPC_IO_BUF_SIZE){
            nbytes = OMRPC_IO_BUF_SIZE;
        }
        r = fread(hp->buf,1, nbytes, tmpfp);
        if(nbytes != r){
            omrpc_fatal("omrpc_send_file: read fail r=%d",r);
        }
        hp->pos = nbytes;
        len -= nbytes;
        omrpc_io_handle_flush(hp);
        if(omrpc_debug_flag){
            omrpc_prf("omrpc_send_filepointer: send flush: rest = %d\n",len);
        }
    }

    /* post proceeding */
    if(omrpc_is_client){
        fflush(tmpfp);
        fclose(tmpfp);
    } else {
        fflush(tmpfp);
        fclose(tmpfp);
    }
}

void omrpc_recv_filename(omrpc_io_handle_t *hp, char **fname)
{
    FILE *tmpfp;
    int len;
    int nbytes;
    int r; 

    /* pre-procedure  */
    if(omrpc_is_client){
        if(*fname == NULL){
            tmpfp = omrpc_make_tmp_fp_filename("w+", fname);
            if(tmpfp == NULL){
                omrpc_error("omrpc_recv_filename: cannot open file");
            }
        } else {
            tmpfp = fopen(*fname, "w+");
        }
    } else {
        tmpfp = omrpc_make_tmp_fp_filename("w+", fname);
        if(tmpfp == NULL){
            omrpc_error("omrpc_recv_filename: cannot open file");
        }
    }

    if(omrpc_debug_flag){
        omrpc_prf("temporary filename : %s\n", *fname);
    }

    /* get file length */
    omrpc_recv_int(hp, &len, 1);
    if (omrpc_debug_flag){
        omrpc_prf("omrpc_recv_filepoiner: filesize=%d\n", len);
    }

    /* the filepointer / finename was null */
    if (len == -1){
        tmpfp = NULL;
        return;
    }

    /* write out current buffer
     * and send file to other side host
     */
    nbytes = hp->ncount - hp->pos;
    if(nbytes > len){
        nbytes = len;
    }
    if(nbytes > 0){
        r = fwrite(hp->buf + hp->pos, 1, nbytes, tmpfp);
        if(r != nbytes){
            omrpc_fatal("omrpc_recv_file: write fail r=%d",r);
        }
        len -= nbytes;
        hp->pos += nbytes;
    }

    while(len > 0){
        omrpc_io_handle_fill(hp);
        if(hp->ncount == 0){
            omrpc_fatal("omrpc_recv_file: unexpected EOF");
        }
        nbytes = len;
        if(nbytes > hp->ncount){
            nbytes = hp->ncount;
        }
        r = fwrite(hp->buf, 1, nbytes, tmpfp);
        if(r != nbytes){
            omrpc_fatal("omrpc_recv_file: write fail r=%d",r);
        }
        len -= nbytes;
        hp->pos = nbytes;
    }

    /* post proceeding */
    if(omrpc_is_client){
        fflush(tmpfp);
        fclose(tmpfp);
    } else {
        fflush(tmpfp);
        fclose(tmpfp);
    }
}


void omrpc_send_recv_filename(omrpc_io_handle_t *hp, char **dp,
                              int count, int stride, int rw)
{
    int i;
    if(stride <= 1){
        if(rw) { /* recv */
            for(i = 0; i < count; i++){
                omrpc_recv_filename(hp, &(dp[i]));
            }
        } else {
            for(i = 0; i < count; i++){
                omrpc_send_filename(hp, &(dp[i]));
            }
        }
    } else {
        omrpc_fatal("no stride (%d) is supported in filename send/recv",
                    stride);
    }
}



void omrpc_io_fileio_unlink_files()
{
    omrpc_io_fileio_name_t *p;
    for(p = omrpc_fileio_name_head; p != NULL; p = p->next){
        omrpc_unlink_tmp_file_name(p->name);
    }
}
