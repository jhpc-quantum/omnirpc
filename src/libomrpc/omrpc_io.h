/*
 * $Id: omrpc_io.h,v 1.2 2006-01-25 16:06:18 ynaka Exp $
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
#ifndef _OMRPC_IO_H_
#define _OMRPC_IO_H_

#include <sys/types.h>
#include <pthread.h>
#include "omni_platform.h"

#define MAX_HANDLE_PER_PORT     128     /* max handle per port if multiplex */
#define TIMEOUT_SEC   5

#define OMRPC_IO_R 1
#define OMRPC_IO_W 0

#define WORKING_PATH_DEFAULT  "/tmp/"

/*
 * RPC I/O operation
 */
#define OMRPC_TIMEOUT_SEC_DEFAULT  15
extern int omrpc_timeout_sec;

#define OMRPC_IO_BUF_SIZE 1024  /* 1K */

typedef struct omrpc_io_handle {
    struct omrpc_io_port *port; /* port for this handle */
    unsigned short int port_n;  /* port number or channel number */
    short int ncount;           /* in bytes */
    char swap_flag;             /* swap for endian */
    char peeked;
    short int peeked_ncount;
    int pos;                    /* current pointer */
    void *hint;                 /* hint, used for store rpc object */
    char buf[OMRPC_IO_BUF_SIZE];
} omrpc_io_handle_t;

/* port type */
#define PORT_SINGLE             0
#define PORT_MXIO               1
#define PORT_GLOBUS             2
#define PORT_GLOBUS_MXIO        3

/* this structure is allocated per port or globus handle */
typedef struct omrpc_io_port {
    char port_type;            /* PORT_SINGLE */
    struct omrpc_io_port *next,*prev;   /* link */
    int fd;                    /* for socket (default) */

    struct omrpc_io_handle *hp;
} omrpc_io_port_t;

typedef struct omrpc_io_port_mx {
    char port_type;             /* PORT_MXIO */
    struct omrpc_io_port *next,*prev;   /* link */
    int fd;                     /* for socket  */

    /* lock for waiting port 0 */
    char swap_flag;
    char active_flag;
    pthread_mutex_t lock;
    pthread_cond_t cond;

    struct omrpc_io_handle *handles[MAX_HANDLE_PER_PORT];
} omrpc_mxio_port_t;

/* list for file list */
typedef struct omrpc_io_fileio_name {
    char *name; /* filename */
    struct omrpc_io_fileio_name *next, *prev; 
    char mode;
} omrpc_io_fileio_name_t;

extern omrpc_io_fileio_name_t *omrpc_fileio_name_head;
extern omrpc_io_fileio_name_t *omrpc_fileio_name_tail;

#define OMRPC_IO_OK     0
#define OMRPC_IO_ERROR (-1)

/* client hostname */
extern char *omrpc_my_hostname;

/* port list */
extern omrpc_io_port_t *omrpc_port_head,*omrpc_port_tail;

/* working directory */
extern char *omrpc_working_path;

/* prototype in this module */
void omrpc_io_init(void);
void omrpc_io_finalize(void);

/* low-level I/O on socket */
int omrpc_io_socket(unsigned short *port);
int omrpc_io_accept(int);
int omrpc_io_connect(char *host, unsigned short port);

omrpc_io_handle_t *omrpc_io_handle_fd(int fd,int mxio_flag);

void omrpc_io_event_init(void);
omrpc_io_handle_t *omrpc_io_handle_create(omrpc_io_handle_t *hp);
int omrpc_wait_any_input(void);
omrpc_io_handle_t *omrpc_get_input_handle(void);
void omrpc_io_modified(void);

void omrpc_io_lock(void);
void omrpc_io_unlock(void);

short omrpc_io_handle_port_num(omrpc_io_handle_t *hp);

void omrpc_read_mxio_header(omrpc_mxio_port_t *port,
                            int *port_n, int *nbytes);
void omrpc_write_mxio_header(omrpc_mxio_port_t *port,
                             int port_n, int nbytes);
void omrpc_send_mxio_ack(omrpc_mxio_port_t *port,char cmd);

int omrpc_read_nbytes(int fd, char *cp, int nbytes);
int omrpc_write_nbytes(int fd, char *cp, int nbytes);

void omrpc_io_handle_close(omrpc_io_handle_t *hp);
void omrpc_io_handle_accept(omrpc_io_handle_t *hp);

/* prototype omrpc_io_rw.c */
void omrpc_send_done(omrpc_io_handle_t *p);
void omrpc_recv_done(omrpc_io_handle_t *p);
void omrpc_recv_done0(omrpc_io_handle_t *p);

void omrpc_send_cmd(omrpc_io_handle_t *p,char cmd);
char omrpc_recv_cmd(omrpc_io_handle_t *p);

void omrpc_send_byte(omrpc_io_handle_t *p,char *dp,int count);
void omrpc_recv_byte(omrpc_io_handle_t *p,char *dp,int count);

void omrpc_send_char(omrpc_io_handle_t *p,char *dp,int count);
void omrpc_recv_char(omrpc_io_handle_t *p,char *dp,int count);
void omrpc_send_recv_char(omrpc_io_handle_t *p, char *dp,int count,
                          int stride, int rw);
void omrpc_send_u_char(omrpc_io_handle_t *p,u_char *dp,int count);
void omrpc_recv_u_char(omrpc_io_handle_t *p,u_char *dp,int count);
void omrpc_send_recv_u_char(omrpc_io_handle_t *p, u_char *dp,int count,
                            int stride, int rw);
void omrpc_send_short(omrpc_io_handle_t *p,short *dp,int count);
void omrpc_recv_short(omrpc_io_handle_t *p,short *dp,int count);
void omrpc_send_recv_short(omrpc_io_handle_t *p, short *dp,int count,
                           int stride, int rw);
void omrpc_send_u_short(omrpc_io_handle_t *p,u_short *dp,int count);
void omrpc_recv_u_short(omrpc_io_handle_t *p,u_short *dp,int count);
void omrpc_send_recv_u_short(omrpc_io_handle_t *p, u_short *dp,int count,
                             int stride, int rw);
void omrpc_send_int(omrpc_io_handle_t *p,int *dp,int count);
void omrpc_recv_int(omrpc_io_handle_t *p,int *dp,int count);
void omrpc_send_recv_int(omrpc_io_handle_t *p,int *dp,int count,
                         int stride, int rw);
void omrpc_send_u_int(omrpc_io_handle_t *p,u_int *dp,int count);
void omrpc_recv_u_int(omrpc_io_handle_t *p,u_int *dp,int count);
void omrpc_send_recv_u_int(omrpc_io_handle_t *p, u_int *dp,int count,
                           int stride, int rw);
void omrpc_send_long(omrpc_io_handle_t *p,long *dp,int count);
void omrpc_recv_long(omrpc_io_handle_t *p,long *dp,int count);
void omrpc_send_recv_long(omrpc_io_handle_t *p, long *dp,int count,
                          int stride, int rw);
void omrpc_send_u_long(omrpc_io_handle_t *p,u_long *dp,int count);
void omrpc_recv_u_long(omrpc_io_handle_t *p,u_long *dp,int count);
void omrpc_send_recv_u_long(omrpc_io_handle_t *p, u_long *dp,int count,
                            int stride, int rw);
void omrpc_send_long_long(omrpc_io_handle_t *p,long long *dp,int count);
void omrpc_recv_long_long(omrpc_io_handle_t *p,long long *dp,int count);
void omrpc_send_recv_long_long(omrpc_io_handle_t *p, long long *dp,int count,
                               int stride, int rw);
void omrpc_send_u_long_long(omrpc_io_handle_t *p,unsigned long long *dp,
                            int count);
void omrpc_recv_u_long_long(omrpc_io_handle_t *p,unsigned long long *dp,
                            int count);
void omrpc_send_recv_u_long_long(omrpc_io_handle_t *p, unsigned long long *dp,
                                 int count,int stride, int rw);
void omrpc_send_float(omrpc_io_handle_t *p,float *dp,int count);
void omrpc_recv_float(omrpc_io_handle_t *p,float *dp,int count);
void omrpc_send_recv_float(omrpc_io_handle_t *p, float *dp,int count,
                           int stride, int rw);
void omrpc_send_double(omrpc_io_handle_t *p,double *dp,int count);
void omrpc_recv_double(omrpc_io_handle_t *p,double *dp,int count);
void omrpc_send_recv_double(omrpc_io_handle_t *p, double *dp,int count,
                            int stride, int rw);
void omrpc_send_long_double(omrpc_io_handle_t *p,long double *dp,int count);
void omrpc_recv_long_double(omrpc_io_handle_t *p,long double *dp,int count);
void omrpc_send_recv_long_double(omrpc_io_handle_t *p,long double *dp,int count,
                                 int stride, int rw);
void omrpc_send_float_complex(omrpc_io_handle_t *p,float _Complex *dp,int count);
void omrpc_recv_float_complex(omrpc_io_handle_t *p,float _Complex *dp,int count);
void omrpc_send_recv_float_complex(omrpc_io_handle_t *p, float _Complex *dp,int count,
                                   int stride, int rw);
void omrpc_send_double_complex(omrpc_io_handle_t *p,double _Complex *dp,int count);
void omrpc_recv_double_complex(omrpc_io_handle_t *p,double _Complex *dp,int count);
void omrpc_send_recv_double_complex(omrpc_io_handle_t *p,double _Complex *dp,int count,
                                    int stride, int rw);
void omrpc_send_string(omrpc_io_handle_t *p,char *cp);
void omrpc_recv_string(omrpc_io_handle_t *p,char *cp);
void omrpc_recv_strdup(omrpc_io_handle_t *p,char **cpp);

void omrpc_io_handle_byte_order(omrpc_io_handle_t *hp,int is_client);
void omrpc_swap_short(short *sp, int n);
void omrpc_swap_int(int *ip, int n);
void omrpc_swap_long_long(long long *lp, int n);
void omrpc_swap_float(float *fp, int n);
void omrpc_swap_double(double *dp, int n);
void omrpc_swap_long_double(long double *dp, int n);
void omrpc_swap_float_complex(float _Complex *fcp, int n);
void omrpc_swap_double_complex(double _Complex *dcp, int n);
void omrpc_send_recv_string(omrpc_io_handle_t *p, char **dp,int count,
                            int stride, int rw);

void omrpc_send_file(omrpc_io_handle_t *hp,int fd);
void omrpc_recv_file(omrpc_io_handle_t *hp,int fd);

void omrpc_recv_done_port(omrpc_io_port_t *port);
void omrpc_io_handle_fill(omrpc_io_handle_t *hp);
void omrpc_io_handle_flush(omrpc_io_handle_t *hp);
short omrpc_io_handle_port_n(omrpc_io_handle_t *hp);

FILE * omrpc_make_tmp_fp(char *mode);
FILE * omrpc_make_tmp_fp_filename(char *mode, char **filenamep);
void omrpc_send_filepointer(omrpc_io_handle_t *hp, FILE **fpp);
void omrpc_recv_filepointer(omrpc_io_handle_t *hp, FILE **fpp);
void omrpc_send_recv_filepointer(omrpc_io_handle_t *hp, FILE **dp,
                                 int count, int stride, int rw);
void omrpc_send_filename(omrpc_io_handle_t *hp, char **fname);
void omrpc_recv_filename(omrpc_io_handle_t *hp, char **fname);
void omrpc_send_recv_filename(omrpc_io_handle_t *hp, char **dp,
                              int count, int stride, int rw);

void omrpc_io_fileio_unlink_files();

#endif /* !_OMRPC_IO_H_ */

