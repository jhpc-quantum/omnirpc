/*
 * $Id: omrpc_agent_defs.h,v 1.1.1.1 2004-11-03 21:01:19 yoshihiro Exp $
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

/*
 * omrpc-agent header file
 */

/* remote executables structure */
typedef struct omrpc_rex_proc {
    struct omrpc_rex_proc *next;        /* link */
    int pid;
    int fd;
    int port_n;         /* back index, 0 if not bound */
} omrpc_rex_proc_t;

/* worker host structure  */
typedef struct omrpc_worker_node {
    char *hostname;
    int sh_type;
} omrpc_worker_node_t;

/* port to process mapping */
extern omrpc_rex_proc_t *omrpc_agent_procs[MAX_HANDLE_PER_PORT];

#define JOB_AGENT_FORK 0        /* default */
#define JOB_AGENT_RR   1
#define JOB_AGENT_PBS  2
#define JOB_AGENT_SGE  3
#define JOB_AGENT_MPI  4

#define WORKER_SH_RSH  0
#define WORKER_SH_SSH  1
#define WORKER_SH_MPI  2 

extern int omrpc_agent_job_type;

extern int omrpc_agent_narrow_mode; /* true if see all port */
extern int omrpc_agent_narrow_port_n;

/* prototype */
omrpc_rex_proc_t *
omrpc_agent_submit(char *path, char *client_hostname, int port, int nprocs);
void omrpc_agent_proc_killed(int pid);

void omrpc_agent_mxio_init(omrpc_io_port_t *port);
void omrpc_agent_mxio_recv_check();
omrpc_rex_proc_t *omrpc_agent_mxio_submit(char *path, int port_num);
void omrpc_agent_mxio_recv_widen(void);

void omrpc_agent_sched_init(void);
void omrpc_agent_sched_init_mpi(void);

#ifdef USE_GLOBUS
void omrpc_agent_globus_mxio_init(omrpc_io_port_t *port);
void omrpc_agent_globus_mxio_recv_check(void);
void omrpc_agent_globus_mxio_wakeup(void);
#endif

char *omrpc_agent_get_worker_sh(omrpc_worker_node_t node);
