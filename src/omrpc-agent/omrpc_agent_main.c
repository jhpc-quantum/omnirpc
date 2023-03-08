static char rcsid[] = "$Id: omrpc_agent_main.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $";
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
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_agent.h"
#include "omrpc_stub.h"

#include "omrpc_agent_defs.h"

#ifdef USE_MPI
#include <mpi.h>
#endif /* USE_MPI */

short omrpc_stub_version_major,omrpc_stub_version_minor;
short omrpc_stub_init = 0;
char *omrpc_module_name = "omrpc-agent";
short omrpc_n_entry = 0;
NINF_STUB_INFO *omrpc_stub_info_table[1];

extern int omrpc_mxio_flag;
extern char *omrpc_sched_type;

static char *omrpc_req_name(int req);

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    char request;
    char *path;
    int fd;
    omrpc_io_handle_t *stub_hp;
    char buf[256],*registry_path;

#ifdef USE_MPI    
    MPI_Init(&argc, &argv);
#endif /* USE_MPI */
    /* use same sequence */
    omrpc_stub_INIT(argc,argv);  
    stub_hp = omrpc_stub_hp;

    if(omrpc_mxio_flag){
#ifdef USE_GLOBUS
        if(omrpc_use_globus)
            omrpc_agent_globus_mxio_init(stub_hp->port);
        else
#endif
            omrpc_agent_mxio_init(stub_hp->port);
    }
    if(omrpc_sched_type != NULL){
	if(strcmp(omrpc_sched_type, "pbs") == 0){
            omrpc_agent_job_type = JOB_AGENT_PBS;
        } else if(strcmp(omrpc_sched_type, "sge") == 0){
            omrpc_agent_job_type = JOB_AGENT_SGE;
#ifdef USE_MPI
        } else if(strcmp(omrpc_sched_type, "mpi") == 0){
            omrpc_agent_job_type = JOB_AGENT_MPI;
            omrpc_agent_sched_init_mpi();
#endif /* USE_MPI */
        } else {
            if(strcmp(omrpc_sched_type, "rr") != 0)
                fprintf(stderr, "Unknown scheduler type. Set RR\n");
            omrpc_agent_sched_init();
        }
    }else{
        /* default RR scheduler */
        omrpc_agent_sched_init();
    }
next:
    if(omrpc_mxio_flag){
        if(omrpc_debug_flag) omrpc_prf("mxio_recv_check ...\n");
#ifdef USE_GLOBUS
        if(omrpc_use_globus){
            omrpc_agent_globus_mxio_recv_check();
        } else
#endif
            omrpc_agent_mxio_recv_check();
    }

    if(omrpc_debug_flag) omrpc_prf("rcv_cmd ...\n");


    request = omrpc_recv_cmd(stub_hp);
    if(request == (char)EOF){
      goto exit;
    }

    if(omrpc_debug_flag)
        omrpc_prf("req=%d(%s)\n",request,omrpc_req_name(request));

    switch(request){
    case OMRPC_AGENT_READ_REG:
        omrpc_recv_strdup(stub_hp,&path);
        omrpc_recv_done(stub_hp);
        registry_path = buf;
        if(path == NULL) {
            omrpc_get_default_path(buf,REGISTRY_FILE);
        } else {
            strcpy(buf,path);
            strcat(buf,"/");
            strcat(buf,REGISTRY_FILE);
        }

        if(omrpc_debug_flag){
            omrpc_prf("OMRPC_AGENT_READ_REG: file='%s'\n",registry_path);
        }
        fd = open(registry_path,O_RDONLY);
        if(fd < 0){
            omrpc_send_cmd(stub_hp,OMRPC_ACK_NG);
        } else {
            omrpc_send_cmd(stub_hp,OMRPC_ACK_OK);
            omrpc_send_file(stub_hp,fd);
        }
        omrpc_send_done(stub_hp);
        close(fd);
        if(path != NULL) omrpc_free(path);
        break;

    case OMRPC_AGENT_EXEC:
    {
        omrpc_rex_proc_t *rp;
        short port_num;
        long  nprocs;

        omrpc_recv_strdup(stub_hp,&path);
        omrpc_recv_short(stub_hp,&port_num,1);
        omrpc_recv_long(stub_hp,&nprocs,1);
        omrpc_recv_done(stub_hp);
        if(omrpc_debug_flag){
            omrpc_prf("OMRPC_AGENT_EXEC: file='%s' port=%u\n",path,(unsigned short)port_num);
#ifdef USE_MPI
            if(omrpc_agent_job_type == JOB_AGENT_MPI)
            omrpc_prf("                : MPI Mode (nprocs=%d)\n",nprocs);
#endif /* USE_MPI */
        }

        if(omrpc_mxio_flag){ /* for multi IO */
            rp = omrpc_agent_mxio_submit(path,port_num);
#ifdef USE_GLOBUS
            if(omrpc_use_globus) omrpc_agent_globus_mxio_wakeup();
#endif
        } else {
            /* default action: fork */
            rp = omrpc_agent_submit(path,omrpc_client_hostname,port_num,nprocs);
        }
        if(rp == NULL) omrpc_send_cmd(stub_hp,OMRPC_ACK_NG);
        else omrpc_send_cmd(stub_hp,OMRPC_ACK_OK);
        omrpc_send_done(stub_hp);

        omrpc_free(path);
    }
    break;

    case OMRPC_AGENT_READ_DONE:
        omrpc_recv_done(stub_hp);
        if(omrpc_mxio_flag)  omrpc_agent_mxio_recv_widen();
        else omrpc_fatal("AGENT_READ_DONE, not mxio mode");
        /* not reply */
        break;

    case OMRPC_MRG_KILL:        /* kill omrpc-agent */
        omrpc_recv_done(stub_hp);
        omrpc_io_handle_close(stub_hp);
        goto exit;

    case OMRPC_MRG_INIT:
    default:
      omrpc_fatal("agent, unknown command=%d\n",request);
    }

    /* clean up pid */
    while((pid = wait3(&status,WNOHANG,NULL)) != 0){
        if(pid == -1){
            if (errno == ECHILD) {
                break;
            } else if (errno == EINTR) {
                continue;
            }
            perror("wait3");
            omrpc_fatal("wait3 is failed");
        } else
            omrpc_agent_proc_killed(pid);
    }

    goto next;

exit:
    if(omrpc_debug_flag) omrpc_prf("omrpc-agent is terminated\n");
#ifdef USE_MPI
    if(omrpc_agent_job_type == JOB_AGENT_MPI)  MPI_Finalize();
#endif /* USE_MPI */
    exit(0);
}

static char *omrpc_req_name(int req)
{
    switch(req){
    case OMRPC_MRG_INIT: return "INIT";
    case OMRPC_AGENT_READ_REG: return "READ_REG";
    case OMRPC_AGENT_EXEC: return "EXEC";
    case OMRPC_MRG_KILL: return "KILL";
    case OMRPC_AGENT_READ_DONE: return "READ_DONE";
    default: return "???";
    }
}
