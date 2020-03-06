static char rcsid[] = "$Id: omrpc_exec_lib.c,v 1.1.1.1 2004-11-03 21:01:14 yoshihiro Exp $";
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
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "OmniRpc.h"
#include "omrpc_defs.h"
#include "omrpc_rpc.h"
#include "omrpc_host.h"
#include "omrpc_io.h"

#ifdef USE_GLOBUS
#include "omrpc_globus.h"
#endif

/*
 * OmniRpc API: low-level and non-MT version interface.
 * OmniRPC_local/remote_exec("rpc_program","func",arg1,arg2,...)
 */
int omrpc_use_ssh = TRUE; 

void OmniRpcExecInit(int *argc, char **argv[])
{
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

    memset(newArgs, 0, sizeof(char *) * (*argc));
    newArgs[0] = *args;
    args++;
    n++;
    while (*args != NULL) {
        if (strncmp(*args, "--", 2) == 0) {
            p = *args + 2;
            if (*p != '\0') {
                if (strcmp(p, "debug") == 0) {
                    omrpc_debug_flag = TRUE;
                } else if (strcmp(p, "globus") == 0) {
#ifdef USE_GLOBUS
                    omrpc_use_globus = TRUE;
                    if(omrpc_debug_flag) omrpc_prf("use globus...\n");
#else
                    omrpc_fatal("globus mode is not available");
#endif
                } else if (strcmp(p, "ssh") == 0){
                    omrpc_use_ssh = TRUE;
                    if(omrpc_debug_flag) omrpc_prf("use ssh ...\n");
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

skip:

#ifdef USE_GLOBUS
    if(omrpc_use_globus){
        omrpc_init_globus_io();
        omrpc_init_globus(FALSE);
    }
#endif

    omrpc_io_init();
    omrpc_init_client();
}

void OmniRpcExecFinalize()
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

int OmniRpcExecLocal(char *prog_name, char *func_name, ...)
{
    va_list ap;
    int r;
    omrpc_rpc_t *rp;

    rp = omrpc_exec_on_host(NULL,prog_name);
    if(rp == NULL){
        omrpc_prf("cannot exec prog: %s\n",prog_name);
        exit(1);
    }

    va_start(ap, func_name);
    r = omrpc_exec_rpc(rp,func_name,&ap);
    va_end(ap);

    omrpc_exec_terminate(rp);
    return r;
}

/*
 * This API use username who run the client program when 
 * an authentication on a remote node is requested. 
 */
int OmniRpcExecRemote(char *host_name, char *prog_name, char *func_name, ...)
{
    va_list ap;
    int r;
    omrpc_rpc_t *rp;
    omrpc_host_t *host;

    /* allocate host object */
    host = omrpc_alloc_host();
    host->name = host_name;
    if(omrpc_use_globus)
        host->fork_type = FORK_GRAM;
    else if(omrpc_use_ssh)
        host->fork_type = FORK_SSH;
    else
        host->fork_type = FORK_RSH;

    rp = omrpc_exec_on_host(host,prog_name);
    if(rp == NULL){
        omrpc_prf("cannot exec prog: %s\n",prog_name);
        exit(1);
    }

    va_start(ap, func_name);
    r = omrpc_exec_rpc(rp,func_name,&ap);
    va_end(ap);

    omrpc_exec_terminate(rp);
    return r;
}

/* 
 * User may change user name in order to exec a remote executable
 * on remote node. This API semantics is same of OmniRpcExecRemote().
 */
int OmniRpcExecRemoteByUser(char *host_name, char *user_name, char *prog_name, char *func_name, ...)
{
    va_list ap;
    int r;
    omrpc_rpc_t *rp;
    omrpc_host_t *host;

    /* allocate host object */
    host = omrpc_alloc_host();
    host->name = host_name;
    if(user_name != NULL)
      host->user_name = user_name;

    if(omrpc_use_globus)
        host->fork_type = FORK_GRAM;
    else if(omrpc_use_ssh)
        host->fork_type = FORK_SSH;
    else
        host->fork_type = FORK_RSH;

    rp = omrpc_exec_on_host(host,prog_name);
    if(rp == NULL){
        omrpc_prf("cannot exec prog: %s\n",prog_name);
        exit(1);
    }

    va_start(ap, func_name);
    r = omrpc_exec_rpc(rp,func_name,&ap);
    va_end(ap);

    omrpc_exec_terminate(rp);
    return r;
}

/*
 * This API use username who run the client program when 
 * an authentication on a remote node is requested. 
 */
OmniRpcExecHandle OmniRpcExecOnHost(char *host_name, char *prog_name)
{
    omrpc_rpc_t *rp;
    omrpc_host_t *host;

    /* allocate host object */
    host = omrpc_alloc_host();
    host->name = host_name;

    if(omrpc_use_globus)
        host->fork_type = FORK_GRAM;
    else if(omrpc_use_ssh)
        host->fork_type = FORK_SSH;
    else
        host->fork_type = FORK_RSH;

    rp = omrpc_exec_on_host(host,prog_name);
    return (void *)rp;
}

/* 
 * User may change user name in order to exec a remote executable
 * on remote node. The semantics of OmniRpcExecOnHostByUser is same of
 * OmniRpcExecOnHost().
 */
OmniRpcExecHandle OmniRpcExecOnHostByUser(char *host_name, char *user_name, char *prog_name)
{
    omrpc_rpc_t *rp;
    omrpc_host_t *host;

    /* allocate host object */
    host = omrpc_alloc_host();
    host->name = host_name;
    if(user_name != NULL)
      host->user_name = user_name;

    if(omrpc_use_globus)
        host->fork_type = FORK_GRAM;
    else if(omrpc_use_ssh)
        host->fork_type = FORK_SSH;
    else
        host->fork_type = FORK_RSH;

    rp = omrpc_exec_on_host(host,prog_name);
    return (void *)rp;
}

int OmniRpcExecCall(OmniRpcExecHandle handle, char *func_name, ...)
{
    va_list ap;
    int r;

    va_start(ap, func_name);
    r = omrpc_exec_rpc((omrpc_rpc_t *)handle,func_name,&ap);
    va_end(ap);

    return r;
}

void OmniRpcExecTerminate(OmniRpcExecHandle handle)
{
    omrpc_exec_terminate((omrpc_rpc_t *)handle);
}




