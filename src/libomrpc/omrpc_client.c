/*   $Id: omrpc_client.c,v 1.1.1.1 2004-11-03 21:01:14 yoshihiro Exp $
 *   $Release: omnirpc-2.0.1 $
 *   $Copyright:
 *    OmniRPC Version 1.0
 *    Copyright (C) 2002-2004 HPCS Laboratory, University of Tsukuba.
 *    
 *    This software is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License version
 *    2.1 published by the Free Software Foundation.
 *    
 *    Please check the Copyright and License information in the files named
 *    COPYRIGHT and LICENSE under the top  directory of the OmniRPC Grid PRC 
 *    System release kit.
 *    
 *    
 *    $
 */

#include <stdio.h>
#include <stdlib.h>
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_exec.h"
#include "omrpc_agent.h"
#include "ninf_comm_lib.h"
#ifdef USE_GLOBUS
#include "omrpc_globus.h"
#endif

void omrpc_activate_port(omrpc_io_port_t *port);

void omrpc_init_client()
{
    char *s;
    if((s = (char *)getenv("OMRPC_RSH")) != NULL)
        omrpc_RSH_command = s;

    omrpc_is_client = TRUE;
}

void omrpc_finalize_client()
{
    if(omrpc_tlog_flag) tlog_finalize();
}

/* 
 * protocol for manger 
 */
int omrpc_agent_read_registry(omrpc_io_handle_t *agent_hp,char *path,char **cp)
{
    char ack;

    omrpc_send_cmd(agent_hp,OMRPC_AGENT_READ_REG);
    omrpc_send_string(agent_hp,path);
    omrpc_send_done(agent_hp);
    ack = omrpc_recv_cmd(agent_hp);
    if(ack == OMRPC_ACK_OK)
	omrpc_recv_strdup(agent_hp,cp);
    omrpc_recv_done0(agent_hp);
    return ack;
}

int omrpc_agent_invoke(omrpc_io_handle_t *agent_hp,char *path,short port_num,int nprocs)
{
    int  ack;
    long nprocs0;
    nprocs0=(long )nprocs;

    omrpc_send_cmd(agent_hp,OMRPC_AGENT_EXEC);
    omrpc_send_string(agent_hp,path);
    omrpc_send_short(agent_hp,&port_num,1);
    omrpc_send_long(agent_hp,&nprocs0,1);
    omrpc_send_done(agent_hp);

    ack = omrpc_recv_cmd(agent_hp);
    if(ack == (char)EOF) omrpc_fatal("invoke, unexpected EOF");
    omrpc_recv_done0(agent_hp);		/* don't send DONE if mxio port */
    omrpc_activate_port(agent_hp->port);
    return ack;
}

/* for client */
void omrpc_activate_port(omrpc_io_port_t *port)
{
    switch(port->port_type){
#if 0
    case PORT_MXIO:
    {
	omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)port;

	if(!omrpc_handler_running) return;

	omrpc_io_lock();
	if(!mport->active_flag){
	    mport->active_flag = TRUE;	/* activate it */
	    omrpc_io_modified();
	}
	omrpc_io_unlock();
	break;
    }
#endif
#ifdef USE_GLOBUS
    case PORT_GLOBUS_MXIO:
    case PORT_GLOBUS:
	omrpc_globus_recv_done(port);
	break;
#endif
    }
}

void omrpc_recv_done_port(omrpc_io_port_t *port)
{
    switch(port->port_type){
    case PORT_MXIO:
    {
	omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)port;

	if(omrpc_debug_flag) 
	    omrpc_prf("omrpc_recv_done handler_running=%d\n",
		      omrpc_handler_running);

	if(!omrpc_handler_running) return;

#if 0
	omrpc_io_lock();
	mport->active_flag = TRUE;	/* activate it */
	omrpc_io_modified();
#endif
	omrpc_send_mxio_ack(mport,OMRPC_AGENT_READ_DONE);
#if 0
	omrpc_io_unlock();
#endif
	break;
    }
#ifdef USE_GLOBUS
    case PORT_GLOBUS_MXIO:
    {
	omrpc_globus_mxio_port_t *mport = (omrpc_globus_mxio_port_t *)port;
	omrpc_globus_recv_done(port);
	omrpc_io_lock();
	omrpc_globus_send_mxio_ack(mport,OMRPC_AGENT_READ_DONE);
	omrpc_io_unlock();
	break;
    }
    case PORT_GLOBUS:
	omrpc_globus_recv_done(port);
	break;
#endif
    }
}
