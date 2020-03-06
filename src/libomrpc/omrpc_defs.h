/* 
 * $Id: omrpc_defs.h,v 1.1.1.1 2004-11-03 21:01:14 yoshihiro Exp $
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
#ifndef _OMRPC_DEFS_H_
#define _OMRPC_DEFS_H_
#include "omni_platform.h"
#include "tlog-omrpc.h"

#define TRUE 1
#define FALSE 0

#ifndef OMRPC_OK
#define OMRPC_OK 0
#define OMRPC_ERROR (-1)
#endif

#define OMRPC_STUB_MAGIC 0x828

//#define OMRPC_DIR "/usr/local/omrpc"	/* default install directory */
#define OMRPC_DIR "$(DESTDIR)/home/tsuji/local/MUST3"
#define HOME_REG_DIR ".omrpc_registry"
#define REGISTRY_FILE "stubs"
#define HOST_FILE "hosts.xml"

#define OMRPC_MAX_NAME_LEN 128

extern int omrpc_debug_flag;
extern int omrpc_use_globus;
extern int omrpc_handler_running;
extern int omrpc_is_client;

extern int omprc_mon_flag;

#define OMRPC_TLOG_FNAME "omrpc.log"
extern int omrpc_tlog_flag;

#define OMRPC_WORKDIR "/tmp/"
/* prototype */
void *omrpc_malloc(size_t size);
void omrpc_free(void *p);
void omrpc_fatal(char *msg,...);
void omrpc_error(char *msg,...);
void omrpc_prf(char *msg,...);

void omrpc_init_client();
void omrpc_finalize_client();
void omrpc_get_default_path(char *buf, char *file);

char *omrpc_tempnam(char *base, int32_t id);

#endif /* !_OMRPC_DEFS_H_ */



