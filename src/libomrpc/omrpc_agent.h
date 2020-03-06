/* 
 * $Id: omrpc_agent.h,v 1.1.1.1 2004-11-03 21:01:13 yoshihiro Exp $
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
#ifndef _OMRPC_AGENT_H_
#define _OMRPC_AGENT_H_

#define OMRPC_AGENT_NAME "omrpc-agent"

/* manger protocol */
#define OMRPC_MRG_INIT	   0
#define OMRPC_AGENT_READ_REG 1
#define OMRPC_AGENT_EXEC     2
#define OMRPC_MRG_KILL	   3
#define OMRPC_AGENT_READ_DONE 4

int omrpc_agent_read_registry(omrpc_io_handle_t *agent_hp,char *path,char **cp);
int omrpc_agent_invoke(omrpc_io_handle_t *agent_hp,char *path,short port_num,int nprocs);
int omrpc_agent_read_done(omrpc_io_handle_t *agent_hp);

#endif /* _OMRPC_AGENT_H_ */

