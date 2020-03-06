/* 
 * $Id: omrpc_rpc_flib.c,v 1.1.1.1 2004-11-03 21:01:17 yoshihiro Exp $
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
#include "OmniRpc.h"
#include "omrpc_defs.h"

/* fortran wrapper */
static void omrpc_Fstrcpy(char *cstr, char *fstr)
{
    char *p,*q;
    q = cstr;
    for(p = fstr; *p != '*' && *p != '\0'; p++) *q++ = *p;
    *q = '\0';
}

void omnirpc_init_()
{
    extern int omrpc_fortran_flag;
    OmniRpcInit(NULL,NULL);
    omrpc_fortran_flag = TRUE;
}

void omnirpc_finalize_()
{
    OmniRpcFinalize();
}


/* fortran wrapper */
void omnirpc_call_(char *entry_name,...)
{
    char name[OMRPC_MAX_NAME_LEN];
    va_list ap;
    int r;
    
    omrpc_Fstrcpy(name,entry_name);
    va_start(ap,entry_name);
    r = OmniRpcCallV(name,ap);
    va_end(ap);
}

void omnirpc_call_async_(void **r, char *entry_name,...)
{
    char name[OMRPC_MAX_NAME_LEN];
    va_list ap;
    
    omrpc_Fstrcpy(name,entry_name);
    va_start(ap,entry_name);
    *r = OmniRpcCallAsyncV(name,ap);
    va_end(ap);
}

void omnirpc_wait_(void **req)
{
    OmniRpcWait(*req);
}

void omnirpc_probe_(void **req,int *r)
{
    *r = OmniRpcProbe(*req);
}

void omnirpc_wait_all_(int *n,void **reqs)
{
    OmniRpcWaitAll(*n,reqs);
}

void omnirpc_wait_any_(int *n,void **reqs,int *r)
{
    *r = OmniRpcWaitAny(*n,reqs);
}

/* fortran wrapper */
void omnirpc_create_handle_(void **r,char *host_name, char *module_name)
{
    char *hname;
    char _hname[OMRPC_MAX_NAME_LEN];
    char mname[OMRPC_MAX_NAME_LEN];
    if(*host_name == '*') hname = NULL;
    else {
	omrpc_Fstrcpy(_hname,host_name);
	hname = _hname;
    }
    omrpc_Fstrcpy(mname,module_name);
    *r = OmniRpcCreateHandle(hname,mname);
}

void omnirpc_destroy_handle_(void **handle)
{
    OmniRpcDestroyHandle(*handle);
}

void omnirpc_call_by_handle_(void **handle,char *entry_name,...)
{
    va_list ap;
    char name[OMRPC_MAX_NAME_LEN];
    int r;

    omrpc_Fstrcpy(name,entry_name);
    va_start(ap,entry_name);
    r = OmniRpcCallByHandleV(*handle, name,ap);
    va_end(ap);
}

void omnirpc_call_async_by_handle_(void **r, void **handle,
				   char *entry_name,...)
{
    va_list ap;
    char name[OMRPC_MAX_NAME_LEN];
    omrpc_Fstrcpy(name,entry_name);
    va_start(ap,entry_name);
    *r = OmniRpcCallAsyncByHandleV(*handle,name,ap);
    va_end(ap);
}

void omnirpc_module_init_(char *module_name,...)
{
    va_list ap;
    char name[OMRPC_MAX_NAME_LEN];

    omrpc_Fstrcpy(name,module_name);
    va_start(ap,module_name);
    OmniRpcModuleInitV(name,ap);
    va_end(ap);
}


/*
 * for gcc 
 */

void omnirpc_init__()
{
    extern int omrpc_fortran_flag;
    OmniRpcInit(NULL,NULL);
    omrpc_fortran_flag = TRUE;
}

void omnirpc_finalize__()
{
    OmniRpcFinalize();
}

void omnirpc_call__(char *entry_name,...)
{
    char name[OMRPC_MAX_NAME_LEN];
    va_list ap;
    int r;
    
    omrpc_Fstrcpy(name,entry_name);
    va_start(ap,entry_name);
    r = OmniRpcCallV(name,ap);
    va_end(ap);
}

void omnirpc_call_async__(void **r, char *entry_name,...)
{
    char name[OMRPC_MAX_NAME_LEN];
    va_list ap;
    
    omrpc_Fstrcpy(name,entry_name);
    va_start(ap,entry_name);
    *r = OmniRpcCallAsyncV(name,ap);
    va_end(ap);
}

void omnirpc_wait__(void **req)
{
    OmniRpcWait(*req);
}

void omnirpc_probe__(void **req,int *r)
{
    *r = OmniRpcProbe(*req);
}

void omnirpc_wait_all__(int *n,void **reqs)
{
    OmniRpcWaitAll(*n,reqs);
}

void omnirpc_wait_any__(int *n,void **reqs,int *r)
{
    *r = OmniRpcWaitAny(*n,reqs);
}

void omnirpc_create_handle__(void **r,char *host_name, char *module_name)
{
    char *hname;
    char _hname[OMRPC_MAX_NAME_LEN];
    char mname[OMRPC_MAX_NAME_LEN];
    if(*host_name == '*') hname = NULL;
    else {
	omrpc_Fstrcpy(_hname,host_name);
	hname = _hname;
    }
    omrpc_Fstrcpy(mname,module_name);
    *r = OmniRpcCreateHandle(hname,mname);
}

void omnirpc_destroy_handle__(void **handle)
{
    OmniRpcDestroyHandle(*handle);
}

void omnirpc_call_by_handle__(void **handle,char *entry_name,...)
{
    va_list ap;
    char name[OMRPC_MAX_NAME_LEN];
    int r;

    omrpc_Fstrcpy(name,entry_name);
    va_start(ap,entry_name);
    r = OmniRpcCallByHandleV(*handle, name,ap);
    va_end(ap);
}

void omnirpc_call_async_by_handle__(void **r, void **handle,
				   char *entry_name,...)
{
    va_list ap;
    char name[OMRPC_MAX_NAME_LEN];
    omrpc_Fstrcpy(name,entry_name);
    va_start(ap,entry_name);
    *r = OmniRpcCallAsyncByHandleV(*handle,name,ap);
    va_end(ap);
}

void omnirpc_module_init__(char *module_name,...)
{
    va_list ap;
    char name[OMRPC_MAX_NAME_LEN];

    omrpc_Fstrcpy(name,module_name);
    va_start(ap,module_name);
    OmniRpcModuleInitV(name,ap);
    va_end(ap);
}

/* EOF */
