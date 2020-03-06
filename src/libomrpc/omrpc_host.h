/* 
 * $Id: omrpc_host.h,v 1.3 2006-01-31 17:36:55 ynaka Exp $
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
#ifndef _OMRPC_HOST_H_
#define _OMRPC_HOST_H_

#include "ninf_comm_lib.h"
#include "ninf_stub_info.h"

#define OMRPC_MAX_HOSTS	512
#define OMRPC_MAX_MODULES 256

#define FORK_RSH 0
#define FORK_SSH 1
#define FORK_GRAM 2
#define FORK_DAEMON 3
#define FORK_MPI 4

#define JOB_SCHED_FORK 0
#define JOB_SCHED_RR   1
#define JOB_SCHED_PBS  2
#define JOB_SCHED_SGE  3


/* Omni RPC host data structure */
typedef struct omrpc_host {
    char *name;		/* host name */
    int fork_type;	/* FORK_RSH ... */
    char mxio_flag;	/* connect type (single/multi) */
    int job_sched_type;	/* job scheudler type */

    char *registry_path; /* registry path */
    char *agent_path;    /* agent path */
    char *user_name;     /* user name in agent host. */
    char *working_path;  /* working path which temporary file are saved */
  
    struct omrpc_rpc_object *agent_rp;	/* rpc handle for manager */

    int port_num; /* port number of daemon (Specialized for windows version */ 
    int max_jobs; 	/* maximum jobs */
    int n_jobs;		/* # of current active jobs */

    /* info for MPI-extension */
    int mpi_nodes_total;
    int mpi_nodes_used ;
} omrpc_host_t;

extern int omrpc_n_hosts;
extern omrpc_host_t *omrpc_hosts[OMRPC_MAX_HOSTS];

typedef struct omrpc_module {
    char *name;			/* module name */
    struct omrpc_rex_prog *rex_list;
    int have_init;		/* have initialze */
    struct omrpc_stub_entry *init_stub;
    any_t init_args[MAX_PARAMS];
    int nprocs; /* the number of MPI-processes */
} omrpc_module_t;

typedef struct omrpc_stub_entry {
    struct omrpc_stub_entry *next;      /* link */
    char *entry_name;           /* entry name, key */
    NINF_STUB_INFO *stub_info;  /* cached stub info */
    omrpc_module_t *module;
} omrpc_stub_entry_t;

typedef struct omrpc_rex_prog {
    struct omrpc_rex_prog *next;	/* link */
    omrpc_host_t *host;
    char *path;
} omrpc_rex_prog_t;

omrpc_host_t *omrpc_alloc_host();

void omrpc_init_hosts(void);
void omrpc_read_hostfile(char *host_file);
omrpc_module_t *omrpc_find_module(char *module_name);
omrpc_stub_entry_t *omrpc_find_stub_entry(char *entry_name, 
                                          char *module_name);
int omrpc_find_host_id(char *host_name);
omrpc_host_t *omrpc_find_host(char *host_name);
int omrpc_host_has_module(int host_id, omrpc_module_t *module);
char *omrpc_rex_prog_path(omrpc_host_t *host, omrpc_module_t *module);

char *omrpc_job_sched_type_name(int job_sched_type);
void omrpc_release_job(int host_id);
int omrpc_get_host_id(omrpc_host_t *host);

void omrpc_parse_registry(int host_id,char *cp);

#endif /* !_OMRPC_HOST_H_ */





