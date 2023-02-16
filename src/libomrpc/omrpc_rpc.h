/* 
 * $Id: omrpc_rpc.h,v 1.2 2006-01-25 16:06:18 ynaka Exp $
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
#ifndef _OMRPC_RPC_H_
#define _OMRPC_RPC_H_
#include <stdarg.h>
#include <pthread.h>
#include "omrpc_io.h"
#include "omrpc_host.h"
#ifdef USE_MPI
#include "omrpc_mpi_io.h"
#else
typedef void *omrpc_mpi_handle_t;
#endif /* USE_MPI */
#include "ninf_comm_lib.h"
#include "ninf_stub_info.h"

/* remote executable management */
/* this object is returned for object handle */
typedef struct omrpc_rpc_object {
    struct omrpc_rpc_object *prev,*next;
    char *prog_name;		/* rpc executable name */
    omrpc_module_t *module;
    omrpc_io_handle_t *hp;	/* I/O handle to communcate */
    omrpc_mpi_handle_t *mp;     /* MPI handler */
    omrpc_host_t *host;		/* host on which this rpc is executed */
    short ver_major,ver_minor;	/* version number of this rpc */
    short have_init;
    short n_entry;
    short id;			/* id for this RPC */

    int is_handle;		/* used as handle */

    struct omrpc_request *req;	/* link to request */

    int pid;	/* pid if 'exec_on_host' */
    void *job_info; /* for GRAM */

    int nprocs;  /* for MPI_Comm_spawn */
} omrpc_rpc_t;

extern int omrpc_rpc_counter;

#define OMRPC_INVOKE	0
#define OMRPC_STUB_INFO	1
#define OMRPC_CALL	2
#define OMRPC_CALL_INIT	3

/* management for outstanding request */
/* this object is returned for synchronization. */
typedef struct omrpc_request {
    struct omrpc_request *next;		/* for free link */
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int done_flag;

    int kind;		/* reqeust kind */
    struct omrpc_rpc_object *rp; /* pointer to rpc object */
    omrpc_stub_entry_t *stub;
    any_t args[MAX_PARAMS];
  //    va_list *app;
  va_list app;
} omrpc_request_t; 

typedef struct rpc_queue {
    omrpc_rpc_t *head;
    omrpc_rpc_t *tail;
} rpc_queue_t;

void omrpc_rpc_init(void);
void omrpcm_init_queue(void);

omrpc_rpc_t *omrpc_exec_on_host(omrpc_host_t *hostp, char *prog_name);
omrpc_rpc_t *omrpc_exec_on_host_daemon(omrpc_host_t *hostp, char *prog_name);
int omrpc_exec_terminate(omrpc_rpc_t *rp);
int omrpc_exec_rpc(omrpc_rpc_t *rp,char *fname, va_list *app);

int omrpc_req_stub_info_local(omrpc_io_handle_t *hp,char *name);
int omrpc_req_stub_info_local_by_index(omrpc_io_handle_t *hp,short index);
int omrpc_recv_stub_info_local(omrpc_io_handle_t *hp,NINF_STUB_INFO  **spp);

int omrpc_req_stub_info(omrpc_io_handle_t *hp,char *name);
int omrpc_req_stub_info_by_index(omrpc_io_handle_t *hp,short index);
int omrpc_recv_stub_info(omrpc_io_handle_t *hp,NINF_STUB_INFO  **spp);

void omrpc_recv_init_msg(omrpc_io_handle_t *hp,omrpc_rpc_t *rp);

omrpc_rpc_t *omrpc_allocate_rpc_obj(void);
void omrpc_scheduler_init(void);
omrpc_rpc_t *omrpc_schedule_rpc(omrpc_module_t *module);
omrpc_rpc_t *omrpc_allocate_rpc_on_host(omrpc_module_t *module,
					char *host_name);
void omrpc_release_jobs(int host_id);

omrpc_rpc_t *omrpc_get_idle_rpc(omrpc_module_t *module);
omrpc_rpc_t *omrpc_get_victim_idle_rpc(omrpc_host_t *host,
				       omrpc_module_t *module);
omrpc_rpc_t *omrpcm_get_victim_idle_rpc(omrpc_host_t *host,
                                        omrpc_module_t *module);
omrpc_rpc_t *omrpc_exec_module(omrpc_host_t *host,omrpc_module_t *module);
omrpc_request_t *omrpc_call_submit(omrpc_rpc_t *rp,
				   omrpc_stub_entry_t *sp,va_list *app);

void omrpc_rpc_handler(omrpc_request_t *req);
int  omrpc_request_wait_done(omrpc_request_t *req);
void omrpc_put_idle_rpc(omrpc_rpc_t *rp);

void omrpc_module_init_setup(omrpc_module_t *mp,va_list *app);
void omrpc_kill_rpc(omrpc_rpc_t *rp);

int omrpc_request_wait_any(int n, omrpc_request_t **req);

void omrpc_wait_jobs();

omrpc_rpc_t *omrpc_new_rpc(void);
omrpc_request_t *omrpc_new_req(omrpc_rpc_t *rp,int req_kind);
void omrpc_delete_rpc(omrpc_rpc_t *rp);

#ifdef USE_FT
int omrpc_check_handle(omrpc_rpc_t *rp);
#endif

#endif /* !_OMRPC_RPC_H_ */


