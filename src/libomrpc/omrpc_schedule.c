/* 
 * $Id: omrpc_schedule.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $
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
#include <pthread.h>
#include "omrpc_defs.h"
#include "omrpc_host.h"
#include "omrpc_rpc.h"

/* 
 *  scheduler: simple scheduler
 */

static pthread_mutex_t sched_lock;
static pthread_cond_t sched_cond;
static int waiting_host = 0;
static int last_sched_host_id;

/* prototype */
static int omrpc_get_last_sched_host();
static void omrpc_set_last_sched_host(int host_id);
static int omrpc_allocate_jobs(int host_id);

void omrpc_scheduler_init()
{
    /* initialize mutex */
    pthread_mutex_init(&sched_lock,NULL);
    pthread_cond_init(&sched_cond,NULL);
    last_sched_host_id = omrpc_n_hosts-1;
}

/*
 * simple round-robin scheduler:
 *   return rcp object 
 */
omrpc_rpc_t *omrpc_schedule_rpc(omrpc_module_t *module)
{
    int id,i,status;
    int n_procs_killed, n_procs_req;
    omrpc_host_t *hp = NULL;
    omrpc_rpc_t *rp;
    int last_host, candidate_host;

again:
    /* find IDLE rpc object */
    if((rp = omrpc_get_idle_rpc(module)) != NULL)
        return rp;

    /* if not found, find available host or candidate */
    candidate_host = -1;
    last_host = omrpc_get_last_sched_host();
    id = (last_host+1) % omrpc_n_hosts;
    for(i = 0; i < omrpc_n_hosts; i++){
        if (omrpc_debug_flag) omrpc_prf("check host id = %d\n",id);
        hp = omrpc_hosts[id];
        if(omrpc_host_has_module(id,module)){
            if(hp->fork_type!=FORK_MPI) status = omrpc_allocate_jobs(id);
            else                        status = omrpcm_allocate_jobs(id, module->nprocs);
            if(status){      /* sucess */
                omrpc_set_last_sched_host(id);
                rp = omrpc_exec_module(hp,module);
                return rp;
            }
            if(candidate_host < 0) candidate_host = id;
        }
        if(++id >= omrpc_n_hosts)  id = 0;
    }

    if(candidate_host < 0)
        omrpc_fatal("no host found for module '%s'\n",module->name);

    if(hp->fork_type!=FORK_MPI){
      if((rp = omrpc_get_victim_idle_rpc(omrpc_hosts[candidate_host],module))!= NULL){
          if (omrpc_debug_flag)
              omrpc_prf("get victim idle rpc host id = %d\n", candidate_host);
          omrpc_kill_rpc(rp);
          omrpc_set_last_sched_host(candidate_host);
          rp = omrpc_exec_module(hp,module);
          return rp;
      }
    }else{
      n_procs_killed = 0;
      n_procs_req    = (hp->mpi_nodes_used+module->nprocs)-hp->mpi_nodes_total;
      while(n_procs_killed<n_procs_req || hp->n_jobs>=hp->max_jobs){
        if((rp = omrpcm_get_victim_idle_rpc(omrpc_hosts[candidate_host],module))!= NULL){
          if (omrpc_debug_flag)
 	    omrpc_prf("get victim idle rpc host id = %d, n_killed=%d, n_req=%d\n",candidate_host,rp->nprocs,n_procs_req);
          n_procs_killed     += rp->nprocs;
          hp->mpi_nodes_used -= rp->nprocs;
          omrpc_kill_rpc(rp);
	  rp->nprocs=0;
        }else{
          break; // go to omrpc_wait_jobs();
        }
      } // while
      if(n_procs_killed>=n_procs_req && rp!=NULL){
	omrpc_set_last_sched_host(candidate_host);
	rp = omrpc_exec_module(hp,module);
	hp->mpi_nodes_used += module->nprocs;
	return rp;
      }
    }    
    omrpc_wait_jobs();
    goto again;
}

omrpc_rpc_t *omrpc_allocate_rpc_on_host(omrpc_module_t *module,
                                        char *host_name)
{
    int id, status;
    omrpc_host_t *hp;
    omrpc_rpc_t *rp;

    id = omrpc_find_host_id(host_name);
    hp = omrpc_hosts[id];
    if(!omrpc_host_has_module(id, module)) return NULL;
again:

    if(hp->fork_type!=FORK_MPI) status = omrpc_allocate_jobs(id);
    else                        status = omrpcm_allocate_jobs(id, module->nprocs);
    if(status){      /* sucess */
        rp = omrpc_exec_module(hp,module);
        return rp;
    }
    if((rp = omrpc_get_victim_idle_rpc(hp,NULL)) != NULL){
        omrpc_kill_rpc(rp);
        rp = omrpc_exec_module(hp,module);
        return rp;
    }
    omrpc_wait_jobs();
    goto again;
}

static int omrpc_get_last_sched_host()
{
    int r;
    pthread_mutex_lock(&sched_lock);
    r = last_sched_host_id;
    pthread_mutex_unlock(&sched_lock);
    return r;
}

static void omrpc_set_last_sched_host(int host_id)
{
    pthread_mutex_lock(&sched_lock);
    last_sched_host_id = host_id;
    pthread_mutex_unlock(&sched_lock);
}

static int omrpc_allocate_jobs(int host_id)
{
    omrpc_host_t *hp;
    int flag = FALSE;

    hp = omrpc_hosts[host_id];
    pthread_mutex_lock(&sched_lock);
    if(hp->n_jobs < hp->max_jobs){
        hp->n_jobs++;
        if(omrpc_debug_flag)
            omrpc_prf("allocate_jobs: host_id=%d\n",host_id);
        flag = TRUE;
    }
    pthread_mutex_unlock(&sched_lock);
    return flag;
}

int omrpcm_allocate_jobs(int host_id, int nprocs)
{
    omrpc_host_t *hp;
    int flag = FALSE;

    hp = omrpc_hosts[host_id];
    pthread_mutex_lock(&sched_lock);
    if(hp->mpi_nodes_used+nprocs<=hp->mpi_nodes_total){
      if(hp->n_jobs < hp->max_jobs){
          hp->n_jobs++;
          hp->mpi_nodes_used+=nprocs;
          if(omrpc_debug_flag)
              omrpc_prf("allocate_jobs: host_id=%d\n",host_id);
          flag = TRUE;
      }
    }
    pthread_mutex_unlock(&sched_lock);
    return flag;
} /* omrpcm_allocate_jobs */

void omrpc_wait_jobs()
{
    pthread_mutex_lock(&sched_lock);
    waiting_host++;
    pthread_cond_wait(&sched_cond,&sched_lock);
    waiting_host--;
    pthread_mutex_unlock(&sched_lock);
}

void omrpc_release_job(int host_id)
{
    omrpc_host_t *hp;

    pthread_mutex_lock(&sched_lock);
    if(host_id >= 0) {
	hp = omrpc_hosts[host_id];
	hp->n_jobs--;
    }
    if(omrpc_debug_flag) 
	omrpc_prf("de-allocate_jobs: host_id=%d\n",host_id);
    if (waiting_host != 0) pthread_cond_signal(&sched_cond);
    pthread_mutex_unlock(&sched_lock);
}


