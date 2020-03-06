/* 
 * $Id: omrpc_globus.h,v 1.1.1.1 2004-11-03 21:01:15 yoshihiro Exp $
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
#ifndef _OMRPC_GLOBUS_H_
#define _OMRPC_GLOBUS_H_

#include "globus_io.h"
#include "globus_gram_client.h"

/* interfaces to GLOBUS */
typedef struct omrpc_globus_io_port {
    char port_type;		/* PORT_GLOBUS */
    globus_io_handle_t g_handle,listener_handle;

    char async_flag;

    struct omrpc_io_handle *hp;
} omrpc_globus_io_port_t;

typedef struct omrpc_globus_mxio_port {
    char port_type;		/* PORT_GLOBUS_MX */
    globus_io_handle_t g_handle,listener_handle;

    char swap_flag;
    char active_flag;
    globus_mutex_t lock;	/* locking */
    globus_cond_t cond;		/* conditional variable for input for 0 */
    short hbuf[2];		/* buffer for peek header */

    int n_handle;		/* number of handle, expect 0 */
    struct omrpc_io_handle *handles[MAX_HANDLE_PER_PORT];
} omrpc_globus_mxio_port_t;

#define OMRPC_GLOBUS_JOB_MANAGER "jobmanager-fork"

/* prototype */
void omrpc_init_globus(int rpc_flag);
void omrpc_finalize_globus(void);

void omrpc_init_globus_io(void);
void omrpc_finalize_globus_io(void);

omrpc_io_handle_t *omrpc_globus_io_handle(int mxio_flag);
void omrpc_globus_listener(globus_io_handle_t *handle, unsigned short *port);
void omrpc_globus_io_accept(globus_io_handle_t *listener_handle,
			    omrpc_io_handle_t *hp);
void omrpc_globus_io_handle_fill(omrpc_io_handle_t *hp);
void omrpc_globus_io_handle_flush(omrpc_io_handle_t *hp);
void omrpc_globus_io_handle_close(omrpc_io_handle_t *hp);
void omrpc_globus_io_handle_byte_order(omrpc_io_handle_t *hp,int is_client);

void omrpc_globus_read_mxio_header(omrpc_globus_mxio_port_t *port,
				   int *port_n, int *nbytes);
void omrpc_globus_write_mxio_header(omrpc_globus_mxio_port_t *port,
				    int port_n, int nbytes);
void omrpc_globus_send_mxio_ack(omrpc_globus_mxio_port_t *port,char cmd);

omrpc_io_handle_t *omrpc_globus_io_handle_create(omrpc_io_handle_t *hp);
void omrpc_globus_io_handle_accept(omrpc_io_handle_t *hp);

void omrpc_globus_io_connect(char *hostname, unsigned short port_n, 
			     omrpc_io_port_t *port);

void omrpc_globus_recv_done(omrpc_io_port_t *p);

int omrpc_exec_by_gram(char *exec_host,char *path,char *host,
		       unsigned short port_num, int mxio_flag,
		       char *job_sched_type,char *reg_path,
		       void **jobs_info);
#endif
