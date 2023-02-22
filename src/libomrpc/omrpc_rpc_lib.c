/* 
 * $Id: omrpc_rpc_lib.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $
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
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "OmniRpc.h"
#include "omrpc_defs.h"
#include "omrpc_rpc.h"
#ifdef USE_MPI
#include "omrpc_mpi_io.h"
#include "omrpc_mpi.h"
#endif /* USE_MPI */
#include "myx_master_wrapper.h"
#ifdef USE_GLOBUS
#include "omrpc_globus.h"
#endif

void omrpcm_prt_mpi();

extern pthread_cond_t  omrpc_mpi_cond;
extern pthread_mutex_t omrpc_mpi_mutex;

/* 
 * OmniRPC API for Remote Proceure Calls
 */

/*
 * Caller must pass UNMODIFIED argc and argv, just from main().
 */
void OmniRpcInit(int *argc, char **argv[])
{
    char *host_file = NULL;
    extern char *omrpc_my_prog_name;
    int n = 0;
    char **args;
    char *p = NULL;
    char **newArgs;


    if(argc != NULL)
        omrpc_my_prog_name = strdup(*argv[0]);
    else
        omrpc_my_prog_name = "???";

    if (getenv("OMRPC_DEBUG") != NULL) {
        omrpc_debug_flag = TRUE;
    }

    if(argv == NULL || argc == NULL) goto skip;

    args = *argv;
    newArgs = (char **)alloca(sizeof(char *) * (*argc));

    if (omrpc_debug_flag) {
        int i;
        for (i = 0; i < *argc; i++) {
            omrpc_prf("debug: Init args before %d '%s'\n", i, args[i]);
        }
    }

    memset(newArgs, 0, sizeof(char *) * (*argc));
    newArgs[0] = *args;
    args++;
    n++;
    while (*args != NULL) {
        if (strncmp(*args, "--", 2) == 0) {
            p = *args + 2;
            if (*p != '\0') {
                if (strcmp(p, "hostfile") == 0) {
                    args++;
                    if (*args == NULL) {
                        break;
                    }
                    if (**args != '\0') {
                        host_file = *args;
                    }
                } else if (strcmp(p, "debug") == 0) {
                    omrpc_debug_flag = TRUE;
#if 0
                } else if (strcmp(p, "np") == 0){
                    args++;
                    if (*args == NULL){
                        break;
                    }
                    if (**args != '\0'){
                        n_proc = atoi(*args);
                        if(n_nproc <= 0){
                            fprintf(stderr,"--np : bad number of hosts");
                            exit(1);
                        }
                    }
#endif
                }
            }
        } else {
            newArgs[n] = *args;
            n++;
        }
        args++;
    }

    if (n != *argc){
        args = *argv;
        memcpy(&(args[0]), &(newArgs[0]), sizeof(char *) * n);
        *argc = n;
    }

    if (omrpc_debug_flag) {
        int i;
        args = *argv;
        for (i = 0; i < *argc; i++) {
            omrpc_prf("debug: init2 after  %d '%s'\n", i, args[i]);
        }
    }

skip:
    omrpc_read_hostfile(host_file);

    omrpc_io_init();
    omrpc_init_client();

#ifdef USE_GLOBUS
    if(omrpc_use_globus) omrpc_init_globus_io();
#endif

    omrpc_init_hosts();
    omrpc_rpc_init();
    omrpc_io_event_init();
#ifdef USE_GLOBUS
    if(omrpc_use_globus) omrpc_init_globus(TRUE);
#endif
}

#ifdef USE_MPI
/* MPI ONLY VERSION */
void OmniRpcMpiInit(int *argc, char **argv[])
{
    char *host_file = NULL;
    extern char *omrpc_my_prog_name;
    int n = 0;
    char **args;
    char *p = NULL;
    char **newArgs;

    MPI_Init(argc, argv);

    if(argc != NULL)
        omrpc_my_prog_name = strdup(*argv[0]);
    else
        omrpc_my_prog_name = "???";

    if (getenv("OMRPC_DEBUG") != NULL) {
        omrpc_debug_flag = TRUE;
    }

    if(argv == NULL || argc == NULL) goto skip;

    args = *argv;
    newArgs = (char **)alloca(sizeof(char *) * (*argc));

    if (omrpc_debug_flag) {
        int i;
        for (i = 0; i < *argc; i++) {
            omrpc_prf("debug: Init args before %d '%s'\n", i, args[i]);
        }
    }

    memset(newArgs, 0, sizeof(char *) * (*argc));
    newArgs[0] = *args;
    args++;
    n++;
    while (*args != NULL) {
        if (strncmp(*args, "--", 2) == 0) {
            p = *args + 2;
            if (*p != '\0') {
                if (strcmp(p, "hostfile") == 0) {
                    args++;
                    if (*args == NULL) {
                        break;
                    }
                    if (**args != '\0') {
                        host_file = *args;
                    }
                } else if (strcmp(p, "debug") == 0) {
                    omrpc_debug_flag = TRUE;
#if 0
                } else if (strcmp(p, "np") == 0){
                    args++;
                    if (*args == NULL){
                        break;
                    }
                    if (**args != '\0'){
                        n_proc = atoi(*args);
                        if(n_nproc <= 0){
                            fprintf(stderr,"--np : bad number of hosts");
                            exit(1);
                        }
                    }
#endif
                }
            }
        } else {
            newArgs[n] = *args;
            n++;
        }
        args++;
    }

    if (n != *argc){
        args = *argv;
        memcpy(&(args[0]), &(newArgs[0]), sizeof(char *) * n);
        *argc = n;
    }

    if (omrpc_debug_flag) {
        int i;
        args = *argv;
        for (i = 0; i < *argc; i++) {
            omrpc_prf("debug: init2 after  %d '%s'\n", i, args[i]);
        }
    }

skip:
    omrpc_read_hostfile(host_file);

    omrpc_io_init(); // get my host name
    omrpc_init_client();
    
    omrpcm_init_hosts();
    omrpcm_rpc_init();
    omrpcm_io_event_init();

#ifdef USE_FT
#ifdef USE_FJMPI
    omrpc_prf("OmniRPC-MPI with HB, FJMPI\n");
#else
    omrpc_prf("OmniRPC-MPI with HB\n");
#endif
#else
#ifdef USE_FJMPI
    omrpc_prf("OmniRPC-MPI with FJMPI\n");
#else
    omrpc_prf("OmniRPC-MPI without FT\n");
#endif
#endif

} /* OmniRpcMpiInit */
#endif /* USE_MPI */

void OmniRpcFinalize()
{
#ifdef USE_GLOBUS
    if(omrpc_use_globus){
        omrpc_finalize_globus();
        omrpc_finalize_globus_io();
    }
#endif
    omrpc_io_finalize();
    omrpc_finalize_client();
}

#ifdef USE_MPI
void OmniRpcMpiFinalize()
{
    omrpcm_finalize_rexes();
    omrpc_io_finalize();
    omrpc_finalize_client();
    MPI_Finalize();
} /*  OmniRpcMpiFinalize */
#endif /* USE_MPI */

/*
 * RPC interface
 */

int OmniRpcCall(char *entry_name,...)
{
    va_list ap;
    int r;

    va_start(ap,entry_name);
    r = OmniRpcCallV(entry_name,ap);
    va_end(ap);
    return r;
}

int OmniRpcCallV(char *entry_name,va_list ap)
{
    int r;
    omrpc_rpc_t *rp;
    omrpc_stub_entry_t *sp;
    omrpc_request_t *req;
    va_list dup_ap;

    if(omrpc_tlog_flag) tlog_CALL_IN_EVENT();

    /* search stub_entry */
    sp = omrpc_find_stub_entry(entry_name,NULL);
    /* if(sp == NULL) return OMRPC_ERROR_ENTRY_NOT_FOUND; */
    if(sp == NULL){
        omrpc_error("entry '%s' is not found",entry_name);
    }

    /* allocate rpc_object */
    rp = omrpc_schedule_rpc(sp->module);
    // necessary?
    va_copy(dup_ap, ap);
    req = omrpc_call_submit(rp,sp,&dup_ap);
    r = omrpc_request_wait_done(req);
    va_end(ap);
    if(omrpc_tlog_flag) tlog_CALL_OUT_EVENT();
    return r;
}

void *OmniRpcCallAsync(char *entry_name,...)
{
    va_list ap;
    void *r;

    va_start(ap,entry_name);
    r = OmniRpcCallAsyncV(entry_name,ap);
    va_end(ap);
    return r;
}

void *OmniRpcCallAsyncV(char *entry_name,va_list ap)
{
    omrpc_rpc_t *rp;
    omrpc_stub_entry_t *sp;
    omrpc_request_t *req;
    va_list dup_ap;

    if(omrpc_tlog_flag) tlog_CALL_ASYNC_EVENT();

    /* search stub_entry */
    sp = omrpc_find_stub_entry(entry_name,NULL);
    /* if(sp == NULL) return OMRPC_ERROR_ENTRY_NOT_FOUND; */
    if(sp == NULL){
        omrpc_error("entry '%s' is not found",entry_name);
    }
    /* PRC (or MPI with 1-procs ?) */
    sp->module->nprocs = 1;

    /* allocate rpc_object */
    rp = omrpc_schedule_rpc(sp->module);
    rp->nprocs = 1; 
    va_copy(dup_ap, ap);
    req = omrpc_call_submit(rp,sp,&dup_ap);
    va_end(ap);

    return (void *)req;
}

#ifdef USE_MPI
/*********************************************************
 * OmniRpcCallAsyncMPI & OmniRpcCallAsyncVMPI are legacy *
 * functions. Generally, you don't have to use them      *
 *********************************************************/
void *OmniRpcCallAsyncMPI(char *entry_name,int nprocs,...)
{
    va_list ap;
    void *r;

    va_start(ap,nprocs);
    r = OmniRpcCallAsyncVMPI(entry_name,nprocs,ap);
    va_end(ap);
    return r;
}

void *OmniRpcCallAsyncVMPI(char *entry_name,int nprocs,va_list ap)
{
    omrpc_rpc_t *rp;
    omrpc_stub_entry_t *sp;
    omrpc_request_t *req;
    va_list dup_ap;

    if(omrpc_tlog_flag) tlog_CALL_ASYNC_EVENT();

    /* search stub_entry */
    sp = omrpc_find_stub_entry(entry_name,NULL);
    /* if(sp == NULL) return OMRPC_ERROR_ENTRY_NOT_FOUND; */
    if(sp == NULL){
        omrpc_error("entry '%s' is not found",entry_name);
    }

    /* put the number of MPI-procs */
    sp->module->nprocs = nprocs;


    /* allocate rpc_object */
    rp = omrpc_schedule_rpc(sp->module);
    rp->nprocs = nprocs; 
    va_copy(dup_ap, ap);
    req = omrpc_call_submit(rp,sp,&dup_ap);
    va_end(ap);

    return (void *)req;
}

/* MPI communication ONLY */
void *OmniRpcMpiCallAsync(char *entry_name,int nprocs,...)
{
    va_list ap;
    void *r;
    omrpc_rpc_t *rp;
    omrpc_stub_entry_t *sp;
    omrpc_request_t *req;
    va_list dup_ap;

    va_start(ap,nprocs);
    
    if(omrpc_tlog_flag) tlog_CALL_ASYNC_EVENT();
    /* search stub_entry */
    sp = omrpc_find_stub_entry(entry_name,NULL);
    /* if(sp == NULL) return OMRPC_ERROR_ENTRY_NOT_FOUND; */
    if(sp == NULL){
        omrpc_error("entry '%s' is not found",entry_name);
    }
    /* put the number of MPI-procs */
    sp->module->nprocs = nprocs;

    /* allocate rpc_object */
    rp = omrpcm_schedule_rpc(sp->module, nprocs);

    rp->nprocs = nprocs;
    va_copy(dup_ap, ap);
    req = omrpcm_call_submit(rp, sp, &dup_ap);
    va_end(ap);

    r = (void *)req;
    va_end(ap);

    return r;
} /* OmniRpcMpiCallAsync */
#endif /* USE_MPI */

int OmniRpcWait(void *request)
{
    omrpc_request_t *req = (omrpc_request_t *)request;
    int r;

    if(omrpc_tlog_flag) tlog_WAIT_IN_EVENT();

    r = omrpc_request_wait_done(req);

    if(omrpc_tlog_flag) tlog_WAIT_OUT_EVENT();

    return;
}

int OmniRpcProbe(void *request)
{
    if(request == NULL)
        return 0;
    omrpc_request_t *req = (omrpc_request_t *)request;
    return req->done_flag;
}

int OmniRpcWaitAll(int n, void **reqs)
{
    int r,i;

    if(omrpc_tlog_flag) tlog_WAIT_IN_EVENT();
    if(omrpc_debug_flag) omrpc_prf("OmniRpcWaitAll n=%d..\n",n);
    r = 0;
    for(i = 0; i < n; i++){
        r = omrpc_request_wait_done((omrpc_request_t *)reqs[i]);
        if(omrpc_debug_flag && r>=0)
	  omrpc_prf("OmniRpcWaitAll req[%d]=0x%x done...\n",i,reqs[i]);
#ifdef USE_FT
	if(r==-1){
	  if(omrpc_tlog_flag) tlog_WAIT_OUT_EVENT();
	  return -(i+1);
	}
#endif
    }
    if(omrpc_debug_flag) omrpc_prf("OmniRpcWaitAll all done...\n");
    if(omrpc_tlog_flag) tlog_WAIT_OUT_EVENT();

    return 0;
}

int OmniRpcWaitAny(int n, void **reqs)
{
    int r;
    if(omrpc_debug_flag) omrpc_prf("OmniRpcWaitAny n=%d...\n",n);
    r = omrpc_request_wait_any(n,(omrpc_request_t **)reqs);
    return r;
}

/*
 * persistent calls interface
 */

/* return rpc_object */
void *OmniRpcCreateHandle(char *host_name, char *module_name)
{
    omrpc_module_t *module;
    omrpc_rpc_t *rp;

    module = omrpc_find_module(module_name);
    if(module == NULL) return NULL;     /* not found */

    if(host_name == NULL)
        rp = omrpc_schedule_rpc(module);
    else
        rp = omrpc_allocate_rpc_on_host(module,host_name);
    if(rp == NULL) return NULL;

    rp->is_handle = TRUE;
    return (void *)rp;
}

void OmniRpcDestroyHandle(void *handle)
{
    int host_id;
    omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;
    host_id =  omrpc_get_host_id(rp->host);
    omrpc_release_job(host_id);
    omrpc_kill_rpc(rp);
}

void OmniRpcDestroyRequest(void *req0)
{
    int host_id;
    omrpc_request_t *req;
    omrpc_rpc_t *rp;

    req=(omrpc_request_t *)req0;
    rp =req->rp;
    host_id =  omrpc_get_host_id(rp->host);
    omrpc_release_job(host_id);
    omrpc_kill_rpc(rp);
}


int OmniRpcCallByHandle(void *handle,char *entry_name,...)
{
    va_list ap;
    int r;

    va_start(ap,entry_name);
    r = OmniRpcCallByHandleV(handle, entry_name,ap);
    va_end(ap);

    return r;
}

void *OmniRpcCallAsyncByHandle(void *handle,char *entry_name,...)
{
    va_list ap;
    void *r;

    va_start(ap,entry_name);
    r = OmniRpcCallAsyncByHandleV(handle, entry_name,ap);
    va_end(ap);

    return r;
}

int OmniRpcCallByHandleV(void *handle, char *entry_name,va_list ap)
{
    omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;
    int r;
    omrpc_stub_entry_t *sp;
    omrpc_request_t *req;
    va_list dup_ap;

    sp = omrpc_find_stub_entry(entry_name,rp->module->name);
    va_copy(dup_ap, ap);
    req = omrpc_call_submit(rp,sp,&dup_ap);
    r = omrpc_request_wait_done(req);
    va_end(ap);

    return r;
}

void *OmniRpcCallAsyncByHandleV(void *handle, char *entry_name,va_list ap)
{
    omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;

    omrpc_stub_entry_t *sp;
    omrpc_request_t *req;
    va_list dup_ap;
    sp = omrpc_find_stub_entry(entry_name,rp->module->name);
    va_copy(dup_ap, ap);
    req = omrpc_call_submit(rp,sp,&dup_ap);
    va_end(ap);

    return (void *)req;
}


/*
 * ModuleInit interface
 */

int OmniRpcModuleInit(char *module_name,...)
{
    va_list ap;

    va_start(ap, module_name);
    OmniRpcModuleInitV(module_name,ap);
    va_end(ap);
    return 0;
}

void OmniRpcModuleInitV(char *module_name,va_list ap)
{
    omrpc_stub_entry_t *sp;
    omrpc_module_t *mp;
    va_list dup_ap;

    if(omrpc_tlog_flag) tlog_INIT_MODULE_EVENT();


    mp = omrpc_find_module(module_name);

    /* search stub_entry */
    sp = omrpc_find_stub_entry("Initialize",module_name);
    /* if(sp == NULL) return OMRPC_ERROR_ENTRY_NOT_FOUND; */
    if(sp == NULL){
        omrpc_error("Initialize entry is not found");
    }
    mp->init_stub = sp;
    va_copy(dup_ap, ap);
    omrpc_module_init_setup(mp,&dup_ap);
    va_end(ap);
}

int OmniRpcModuleReserve(int nstub)
{

  return 0;
}

#ifdef USE_MPI
void *OmniRpcMpiCallAsyncByHandle(void *handle,int nprocs,char *entry_name,...)
{
    va_list ap;
    void *r;

    va_start(ap,entry_name);
    r = OmniRpcMpiCallAsyncByHandleV(handle,nprocs,entry_name,ap);
    va_end(ap);

    return r;
}

void *OmniRpcMpiCallAsyncByHandleV(void *handle,int nprocs,char *entry_name,va_list ap)
{
    omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;

    omrpc_stub_entry_t *sp;
    omrpc_request_t *req;
    va_list dup_ap;

    sp = omrpc_find_stub_entry(entry_name,rp->module->name);

    sp->module->nprocs=nprocs;
    rp->nprocs=nprocs;

    va_copy(dup_ap, ap);

    req = omrpcm_call_submit(rp,sp,&dup_ap);
    va_end(ap);

    return (void *)req;
}

void *OmniRpcMpiCreateHandle(int nprocs, char *module_name)
{
    omrpc_module_t *module;
    omrpc_rpc_t *rp;

    module = omrpc_find_module(module_name);
    if(module == NULL) return NULL;     /* not found */
    module->nprocs=nprocs;

    rp = omrpcm_schedule_rpc(module,nprocs);
    rp->nprocs=nprocs;

    rp->is_handle = TRUE;

    return (void *)rp;
} /* OmniRpcMpiCreateHandle */

MPI_Comm OmniRpcMpiGetComm(void *handle)
{
  omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;
  return rp->mp->comm;
} /* OmniRpcMpiGetComm */

/* wrapper of MPI_Send */
int OmniRpcMpi_Send(void *buf, int count, MPI_Datatype datatype, 
      int dest, int tag,MPI_Comm comm)
{
  int ierr;

  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);
  
  ierr=MPI_Send(buf,count,datatype,dest,tag,comm);

  pthread_mutex_unlock(&omrpc_mpi_mutex);

  return ierr;
} /* OmniRpcMpi_Send */

/* wrapper of MPI_Recv */
int OmniRpcMpi_Recv(void *buf, int count, MPI_Datatype datatype, 
        int source, int tag,MPI_Comm comm, MPI_Status *status)
{
  int ierr;

  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);
  
  ierr=MPI_Recv(buf,count,datatype,source,tag,comm,status);

  pthread_mutex_unlock(&omrpc_mpi_mutex);

  return ierr;
} /* OmniRpcMpi_Recv */

/* wrapper for MPI_Bcast */
int OmniRpcMpi_Bcast(void *buf, int count, MPI_Datatype datatype,
                               int root, MPI_Comm comm)
{
  int ierr;
  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);

  ierr=MPI_Bcast(buf,count,datatype,root,comm);

  pthread_mutex_unlock(&omrpc_mpi_mutex);

  return ierr;
} /* OmniRpcMpi_Bcast */

int OmniRpcMpi_Test(MPI_Request *request, int *flag, MPI_Status *status)
{
  int ierr;
  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);

  ierr=MPI_Test(request,flag,status);

  pthread_mutex_unlock(&omrpc_mpi_mutex);

  return ierr;
} /* OmniRpcMpi_Test */

#ifdef USE_FT
int OmniRpcMpi_FJMPIRecv(void *buf, int count, MPI_Datatype datatype, 
			 int source, int tag,MPI_Comm comm, MPI_Status *status, char *port)
{
  int ierr;

  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);
  
  ierr=FJMPI_Mswk_recv(buf,count,datatype,source,tag,port,status,comm);

  pthread_mutex_unlock(&omrpc_mpi_mutex);

  return ierr;
} /* OmniRpcMpi_Recv */

int OmniRpcMpi_FJMPISend(void *buf, int count, MPI_Datatype datatype, 
			 int dest, int tag,MPI_Comm comm,char *port)
{
  int ierr;

  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);
 
  ierr=FJMPI_Mswk_send(buf,count,datatype,dest,tag,port,comm);

  pthread_mutex_unlock(&omrpc_mpi_mutex);

  return ierr;
} /* OmniRpcMpi_JFMPISend */

char *OmniRpcMpiGetPort(void *handle)
{
  omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;
  return rp->mp->ft_pname;
} /* OmniRpcMpiGetPort */
#endif

#ifdef USE_FT
// return handle with error when Error is found in OmniRpcWaitAll or OmniRpcWaitAny 
int OmniRpcMpiGetErrorHandleId(int ierr)
{
  return (-ierr-1);
} /* OmniRpcMpiGetErrorHandleId */
#endif

// return 1 (alive), 0 (error)
int OmniRpcMpiCheckHandle(void *handle)
{
  int         ierr;

  if(handle==NULL) return 0; 

#ifdef USE_FT
  omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;
  
  ierr=omrpc_check_handle(rp);
#else
  ierr=1;
#endif

  return ierr;
}
#endif /* USE_MPI */

#ifdef USE_FT
// Send signal to rex to stop heart beat (for test)
int OmniRpcMpiStopHeartBeat(void *handle)
{
  int         ierr;
  omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;

  ierr=omrpcm_stop_handle(rp);

  return ierr;
}
#endif

short OmniRpcGetRPID(void *handle)
{
  omrpc_rpc_t *rp = (omrpc_rpc_t *)handle;
  return rp->id;
} /* OmniRpcGetRPID */
