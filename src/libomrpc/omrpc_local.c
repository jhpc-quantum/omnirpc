/*
 * $Id: omrpc_local.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $
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
#include "omrpc_rpc.h"
#include "ninf_comm_lib.h"
#include "omrpc_exec.h"

#ifdef USE_GLOBUS
#include "omrpc_globus.h"
#endif

/*
 * execute on RPC executables on host, and return its handle.
 *    this is usually used to fork manger process
 */
omrpc_rpc_t *omrpc_exec_on_host_daemon(omrpc_host_t *hostp, char *prog_name)
{
    int fd = 0;
    unsigned short port;
    int pid;
    omrpc_rpc_t *rp;
    omrpc_io_handle_t *hp = NULL;


    fd = omrpc_io_connect(hostp->name, hostp->port_num);

    if(omrpc_debug_flag)
      omrpc_prf("connect host=%d\n",port);

    /* allocate rpc object */
    rp = omrpc_allocate_rpc_obj();


    hp = omrpc_io_handle_fd(fd,FALSE);

    /* set to rpc object */
    rp->prog_name = prog_name;
    rp->hp = hp;
    rp->host = hostp;
    rp->pid = pid;

    //    omrpc_recv_init_msg(hp,rp);

    if(omrpc_debug_flag)
        omrpc_prf("omrpc_exec_on_host: ver %d.%d init=%d\n",
               rp->ver_major,rp->ver_minor,rp->have_init);

    return rp;
}

/*
 * execute on RPC executables on host, and return its handle.
 *    this is usually used to fork manger process
 */
omrpc_rpc_t *omrpc_exec_on_host(omrpc_host_t *hostp, char *prog_name)
{
    int fd = 0;
    unsigned short port;
    int pid;
    omrpc_rpc_t *rp;
    omrpc_io_handle_t *hp = NULL;
#ifdef USE_GLOBUS
    globus_io_handle_t listener_handle;
    void *job_info;
#endif
#ifdef USE_MPI
    MPI_Comm intercomm;
#endif /* USE_MPI */
    
    port = 0; /* ANY */
    if(hostp != NULL && hostp->fork_type == FORK_GRAM){
#ifdef USE_GLOBUS
        omrpc_globus_listener(&listener_handle,&port);
#else
        omrpc_fatal("no globus supported");
#endif
    } else
        fd = omrpc_io_socket(&port);

    if(omrpc_debug_flag) omrpc_prf("exec_on_host=%d\n",port);

    if(hostp == NULL){
        pid = omrpc_exec_by_fork(prog_name,NULL,port,FALSE,NULL);
    } else {
        switch(hostp->fork_type){
        case FORK_RSH:          /* rsh */
            pid = omrpc_exec_by_rsh(hostp->name,hostp->user_name,
                                    prog_name,omrpc_my_hostname,port,
                                    hostp->mxio_flag,FALSE,
                                    omrpc_job_sched_type_name(hostp->job_sched_type),
                                    hostp->registry_path,hostp->working_path);
            break;
        case FORK_SSH:          /* secure shell, using port forwarding */
            pid = omrpc_exec_by_ssh(hostp->name,hostp->user_name,
                                    prog_name,omrpc_my_hostname,port,
                                    hostp->mxio_flag,FALSE,
                                    omrpc_job_sched_type_name(hostp->job_sched_type),
                                    hostp->registry_path, hostp->working_path);
            break;
        case FORK_GRAM:         /* globus GRAM */
#ifdef USE_GLOBUS
            pid = omrpc_exec_by_gram(hostp->name,prog_name,
                                     omrpc_my_hostname,port,
                                     hostp->mxio_flag,
                                     omrpc_job_sched_type_name(hostp->job_sched_type),
                                     hostp->registry_path,
                                     &job_info);
            break;
#endif
#ifdef USE_MPI
        case FORK_MPI:         /* MPI_Comm_Spawn */
            pid = omrpc_exec_by_mpi(prog_name,omrpc_my_hostname,port,hostp->working_path,1,"mpi",&intercomm);
            break;
#endif /* USE_MPI */
	    
        default:

            return NULL;        /* unknown type */
        }
    }

    if(pid < 0){
        perror("fork filed:");
        exit(1);
    }

    /* allocate rpc object */
    rp = omrpc_allocate_rpc_obj();

    if(hostp != NULL && hostp->fork_type == FORK_GRAM){
#ifdef USE_GLOBUS
        hp = omrpc_globus_io_handle(hostp->mxio_flag);
        omrpc_globus_io_accept(&listener_handle,hp);
        rp->job_info = job_info;
#endif
    } else {
        /* accept */
        fd = omrpc_io_accept(fd);

        /* create io_handle */
        if(hostp != NULL && hostp->mxio_flag)
            hp = omrpc_io_handle_fd(fd,TRUE);
        else
            hp = omrpc_io_handle_fd(fd,FALSE);
    }

    /* set to rpc object */
    rp->prog_name = prog_name;
    rp->hp = hp;
    rp->host = hostp;
    rp->pid = pid;
    rp->nprocs = 1;

    omrpc_recv_init_msg(hp,rp);

    if(omrpc_debug_flag)
        omrpc_prf("omrpc_exec_on_host: ver %d.%d init=%d\n",
               rp->ver_major,rp->ver_minor,rp->have_init);

    return rp;
}


void omrpc_recv_init_msg(omrpc_io_handle_t *hp,omrpc_rpc_t *rp)
{
    short magic;
    omrpc_host_t *hostp;

    omrpc_io_handle_byte_order(hp,TRUE);
    /* set version number as the first message */
    omrpc_recv_short(hp,&magic,1);
    if(magic != OMRPC_STUB_MAGIC) 
        omrpc_fatal("bad magic 0x%x != 0x%x",OMRPC_STUB_MAGIC,magic);
    omrpc_recv_short(hp,&rp->ver_major,1);
    omrpc_recv_short(hp,&rp->ver_minor,1);
    omrpc_recv_short(hp,&rp->have_init,1);
    omrpc_recv_short(hp,&rp->n_entry,1);
    omrpc_recv_done(hp);
}

int omrpc_exec_terminate(omrpc_rpc_t *rp)
{
    int status,pid;

    if(omrpc_debug_flag) omrpc_prf("omrpc_send_kill\n");
    /* send kill */
    omrpc_send_kill(rp->hp);

    if(rp->pid != 0) kill(rp->pid,1);	/* force to kill */

    /* wait for kill */
again:
    if((pid = waitpid(rp->pid, &status, 0)) < 0) return(-1);
    if(WIFSTOPPED(status)) goto again;

    if(pid != rp->pid) {
        omrpc_fatal("bad wait pid : waiting %d, returned %d", rp->pid, pid);
      return -1;
    }

    if(WIFSIGNALED(status)) return(-1);
    else return(WEXITSTATUS(status));
}

/*
 *  call stub with send/recv
 */
int omrpc_exec_rpc(omrpc_rpc_t *rp, char *name, va_list *app)
{
    omrpc_io_handle_t *hp = rp->hp;
    NINF_STUB_INFO  *stub_info;
    any_t args[MAX_PARAMS];

    omrpc_req_stub_info(hp,name);
    if(omrpc_recv_stub_info(hp,&stub_info) != OMRPC_OK)
        return OMRPC_ERROR;

    omrpc_setup_args(app,args,stub_info);

    /* do call */
    omrpc_call_send_args(hp,stub_info,args);
    omrpc_call_recv_args(hp,stub_info,args);

    /* free stub_info */
    ninf_free_stub_info(stub_info);

    return OMRPC_OK;
}

/*
 * get local stub_info in binary form
 */
int omrpc_req_stub_info_local(omrpc_io_handle_t *hp,char *name)
{
    omrpc_send_cmd(hp,OMRPC_REQ_STUB_INFO_LOCAL);
    omrpc_send_string(hp,name);
    omrpc_send_done(hp);
    if(omrpc_debug_flag) omrpc_prf("omrpc_req_stub_info_local...\n");
    return TRUE;
}

int omrpc_req_stub_info_local_by_index(omrpc_io_handle_t *hp,short index)
{
    omrpc_send_cmd(hp,OMRPC_REQ_STUB_INFO_LOCAL_BY_INDEX);
    omrpc_send_short(hp,&index,1);
    omrpc_send_done(hp);
    if(omrpc_debug_flag) omrpc_prf("omrpc_req_stub_info_local...\n");
    return TRUE;
}

int omrpc_recv_stub_info_local(omrpc_io_handle_t *hp,NINF_STUB_INFO  **spp)
{
    char ack;
    NINF_STUB_INFO *sp;
    struct ninf_param_desc *dp;
    int i;

    if(omrpc_debug_flag) omrpc_prf("omrpc_recv_stub_info_local...\n");

    sp = ninf_new_stub_info();

    ack = omrpc_recv_cmd(hp);
    if(ack != OMRPC_ACK_OK){
        omrpc_recv_done(hp);
        return OMRPC_ERROR;
    }

    omrpc_recv_byte(hp,(char *)sp,sizeof(*sp));
    dp = ninf_new_param_desc(sp->nparam);
    sp->params = dp;

    for(i = 0; i < sp->nparam; i++)
        omrpc_recv_byte(hp,(char *)&(sp->params[i]),
                        sizeof(struct ninf_param_desc));
    *spp = sp;
    sp->description = NULL;

    if(omrpc_debug_flag){
        omrpc_prf("get_stub_info:");
        ninf_print_stub_info(stderr,sp);
    }
    omrpc_recv_done(hp);

    return OMRPC_OK;
}






