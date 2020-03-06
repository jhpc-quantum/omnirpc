static char rcsid[] = "$Id: omrpc_agent_job.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $";
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
#include <string.h>
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_exec.h"

#include "omrpc_agent_defs.h"
#include "omrpc_stub_lib.h"

#include "../../include/omrpc_mpi_lib.h"

#define MAXPATHLEN 256
#define MAX_LINE_LEN 4096

/*
 * local job manger interface
 */
omrpc_rex_proc_t *omrpc_agent_procs[MAX_HANDLE_PER_PORT];
int omrpc_agent_job_type = JOB_AGENT_FORK;

extern int omrpc_mxio_flag;
extern char *omrpc_client_hostname;
static omrpc_rex_proc_t *proc_list = NULL;
static omrpc_rex_proc_t *proc_free = NULL;   /* free list */

/* for simple round-robin scheduler */
#define MAX_NODES 128

int n_nodes;
int current_node;
omrpc_worker_node_t nodes_table[MAX_NODES];


static void dump_nodes_list(omrpc_worker_node_t node[], int n);


omrpc_rex_proc_t *
omrpc_agent_submit(char *path, char *client_hostname, int port, int nprocs)
{
    int pid = -1;
    int i,j;
    omrpc_mpi_node_t *worker_nodes;
    omrpc_rex_proc_t *rp;
    MPI_Comm         intercomm;

    switch(omrpc_agent_job_type){
    case JOB_AGENT_FORK:
        pid = omrpc_exec_by_fork(path,client_hostname,port,
                                 omrpc_mxio_flag ? FALSE : omrpc_use_globus,
                                 omrpc_working_path);
        break;
    case JOB_AGENT_RR:
        if(omrpc_debug_flag)
            omrpc_prf("sched rr, node=%s (%d)\n",
                      nodes_table[current_node].hostname,current_node);
        if(nodes_table[current_node].sh_type == WORKER_SH_SSH){
            // this code may be dangerous.
            pid = omrpc_exec_worker_by_ssh(nodes_table[current_node].hostname,
                                           NULL, path,
                                           client_hostname,
                                           port, FALSE,
                                           omrpc_mxio_flag ? FALSE : omrpc_use_globus,
                                           NULL, omrpc_working_path);
        } else {
            pid = omrpc_exec_by_rsh(nodes_table[current_node].hostname, NULL,
                                    path,client_hostname,port,
                                    FALSE,
                                    omrpc_mxio_flag ? FALSE : omrpc_use_globus,
                                    NULL,NULL,omrpc_working_path);
        }
        if(++current_node >= n_nodes) current_node = 0;
        break;

    case JOB_AGENT_PBS:
        if(omrpc_debug_flag)
            omrpc_prf("sched pbs");
        pid = omrpc_exec_by_pbs(path, client_hostname, port, FALSE,
                                omrpc_mxio_flag ? FALSE : omrpc_use_globus,
                                NULL,omrpc_working_path);
        break;

    case JOB_AGENT_SGE:
        if(omrpc_debug_flag)
            omrpc_prf("sched sge");
        pid = omrpc_exec_by_sge(path, client_hostname, port, FALSE,
                                omrpc_mxio_flag ? FALSE : omrpc_use_globus,
                                NULL,omrpc_working_path);
        break;

    case JOB_AGENT_MPI:
        pid = omrpc_exec_by_mpi(path, client_hostname, port, 
                                omrpc_working_path, nprocs, "mpi", &intercomm);

        worker_nodes=(omrpc_mpi_node_t *)malloc(nprocs*sizeof(omrpc_mpi_node_t));

        omrpc_recv_nodelist(nprocs,intercomm,worker_nodes);
        if(omrpc_debug_flag){
          for(i=0;i<nprocs;i++){
            omrpc_prf("%3d : %3d.%3d.%3d.%3d : %s\n"
               ,worker_nodes[i].rank
               ,worker_nodes[i].ip[0],worker_nodes[i].ip[1],worker_nodes[i].ip[2],worker_nodes[i].ip[3]
               ,worker_nodes[i].name);
          }
        }

        free(worker_nodes);
        break;

    default:
        omrpc_fatal("unknown job agent type %d",omrpc_agent_job_type);
    }

    if(pid < 0) return NULL;    /* failed */

    if(proc_free == NULL){
        rp = (omrpc_rex_proc_t *)omrpc_malloc(sizeof(omrpc_rex_proc_t));
    } else {
        rp = proc_free;
        proc_free = proc_free->next;
        bzero(rp,sizeof(omrpc_rex_proc_t));
    }
    rp->pid = pid;

    /* enqueue */
    rp->next = proc_list;
    proc_list = rp;
    return rp;
}

void omrpc_agent_proc_killed(int pid)
{
    omrpc_rex_proc_t *rp,*rq;

    for(rp = proc_list, rq = NULL; rp != NULL; rq = rp, rp = rp->next)
        if(rp->pid == pid) break;
    if(rp == NULL)
        omrpc_fatal("no pid is found, pid=%d\n",pid);
    if(rq == NULL){
        proc_list = rp->next;
    } else {
        rq->next = rp->next;
    }

    /* if port_n is assgined, remote it */
    if(rp->port_n != 0 && omrpc_agent_procs[rp->port_n] == rp)
        omrpc_agent_procs[rp->port_n] = NULL;

    /* back to free list */
    rp->next = proc_free;
    proc_free = rp;
}

/*
 * simple scheduler for cluster
 *  if file "node" is found in default path, switch to RR
 */
void omrpc_agent_sched_init()
{
    FILE *fp;
    char fname[MAXPATHLEN];
    char linebuf[MAX_LINE_LEN + 1];
    char node_name[MAX_LINE_LEN];
    char shtype[MAX_LINE_LEN];
    char *cp;
    int i;
    extern char *omrpc_agent_registry_path;


    if(omrpc_agent_registry_path != NULL){
      strcpy(fname,omrpc_agent_registry_path);
      strcat(fname,"/nodes");
    } else
      omrpc_get_default_path(fname,"nodes");

    fp = fopen(fname,"r");
    if(fp == NULL){
        if(omrpc_debug_flag) omrpc_prf("nodes file '%s' not found\n",fname);
        return;
    }
    if(omrpc_debug_flag) omrpc_prf("nodes file: '%s'\n",fname);

    n_nodes = 0;
    while(fgets(linebuf,MAX_LINE_LEN,fp) != NULL){
        cp = linebuf;
        /* check the maximum nodes */
        if( n_nodes >= MAX_NODES){
            omrpc_prf("Maximum number of computational nodes is %d, skipping extra nodes..",
                      MAX_NODES);
            n_nodes = MAX_NODES;
            break;
        }

        while(*cp != '\0' && isspace((int)(*cp))) cp++;
        if(*cp == '#' || *cp == '\0' ||
           *cp == '\r' || *cp == '\n')
            continue;
        i = sscanf(cp,"%s%s", node_name, shtype);
        if(i <= 0) omrpc_fatal("bad hosts file '%s'\n",fname);
        /* check the stub invoker */
        if (i == 1) {
            nodes_table[n_nodes].sh_type = WORKER_SH_RSH;
        } else if(i == 2) {
            if(strcmp(shtype, "ssh") == 0) {
                nodes_table[n_nodes].sh_type = WORKER_SH_SSH;
            } else {
                nodes_table[n_nodes].sh_type = WORKER_SH_RSH;
            }
        } else {
            nodes_table[n_nodes].sh_type = WORKER_SH_RSH;
        }
        nodes_table[n_nodes++].hostname = strdup(node_name);
    }
    fclose(fp);
    if(omrpc_debug_flag) dump_nodes_list(nodes_table,n_nodes);
    if(omrpc_debug_flag) omrpc_prf(" %d nodes in '%s'\n",n_nodes,fname);
    if(n_nodes == 0)
      return;
    
    omrpc_agent_job_type = JOB_AGENT_RR;       /* set round robin scheduler */
}

/*
 * scheduler for MPI+OmniRPC
 */
void omrpc_agent_sched_init_mpi()
{
    FILE *fp;
    char fname[MAXPATHLEN];
    char linebuf[MAX_LINE_LEN + 1];
    char node_name[MAX_LINE_LEN];
    char shtype[MAX_LINE_LEN];
    char *cp;
    char flag;
    int  i, *universe_sizep;
    extern char *omrpc_agent_registry_path;

    MPI_Comm_get_attr(MPI_COMM_WORLD, MPI_UNIVERSE_SIZE, &universe_sizep, &flag);
    if(!flag){
      omrpc_prf("This MPI does not support UNIVERSE_SIZE\n");
    }else{
      if(omrpc_debug_flag) omrpc_prf("universe_size = %d\n",*universe_sizep);
    }
    if(omrpc_agent_registry_path != NULL){
      strcpy(fname,omrpc_agent_registry_path);
      strcat(fname,"/nodes");
    } else
      omrpc_get_default_path(fname,"nodes");

    fp = fopen(fname,"r");
    if(fp == NULL){
        if(omrpc_debug_flag) omrpc_prf("nodes file '%s' not found\n",fname);
        return;
    }
    if(omrpc_debug_flag) omrpc_prf("nodes file: '%s'\n",fname);

    n_nodes = 0;
    while(fgets(linebuf,MAX_LINE_LEN,fp) != NULL){
        cp = linebuf;
        /* check the maximum nodes */
        if( n_nodes >= MAX_NODES){
            omrpc_prf("Maximum number of computational nodes is %d, skipping extra nodes..",
                      MAX_NODES);
            n_nodes = MAX_NODES;
            break;
        }
        while(*cp != '\0' && isspace((int)(*cp))) cp++;
        if(*cp == '#' || *cp == '\0' ||
           *cp == '\r' || *cp == '\n')
            continue;
        i = sscanf(cp,"%s%s", node_name, shtype);
        if(i <= 0) omrpc_fatal("bad hosts file '%s'\n",fname);
        /* set the stub invoker, all invoker should be MPI */
        nodes_table[n_nodes].sh_type = WORKER_SH_MPI;
        nodes_table[n_nodes++].hostname = strdup(node_name);
    }
    fclose(fp);
    if(omrpc_debug_flag) dump_nodes_list(nodes_table,n_nodes);
    if(omrpc_debug_flag) omrpc_prf(" %d nodes in '%s'\n",n_nodes,fname);
    if(n_nodes == 0)
      return;
}

static void dump_nodes_list(omrpc_worker_node_t node[], int n)
{
    int i;
    omrpc_prf("nodes:");
    printf("%d nodes\n",n);
    for(i = 0; i < n; i++){
        printf("node %d: %s [%s]\n", i, node[i].hostname,
               omrpc_agent_get_worker_sh(node[i]));
    }
    printf("\n");
    fflush(stdout);
}

char *omrpc_agent_get_worker_sh(omrpc_worker_node_t node)
{
    if(node.sh_type == WORKER_SH_RSH)
        return RSH_COMMAND;
    if(node.sh_type == WORKER_SH_SSH)
        return SSH_COMMAND;
    if(node.sh_type == WORKER_SH_MPI)
        return "mpi";
    else
        return "Unknown";
}
