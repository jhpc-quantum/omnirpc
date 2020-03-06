/* 
 * $Id: omrpc_mon.h,v 1.2 2006-01-25 16:06:18 ynaka Exp $
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

#ifndef _OMRPC_MON_H_
#define _OMRPC_MON_H_H
#include <pthread.h>

#define OMRPC_MONITOR_DEFAULT_PORT 5555
extern int omrpc_monitor_flag;
extern char *omrpc_monitor_target_host;
extern int omrpc_monitor_port;


typedef struct omrpc_monitor_io_handler {
    int fd;                    /* file descriptor */
    struct sockaddr_in sin;    /* monitor target's sockaddr_in */
    struct hostent *hp;        /* monitor target's entity */
} omrpc_monitor_io_handler_t ;

#ifdef USE_MONITOR
void omrpc_monitor_init();
void * omrpc_monitor_handler(void *dummy);
int omrpc_monitor_io_send_msg(omrpc_monitor_io_handler_t *hd, char *msg);
int omrpc_monitor_info_send_by_load(omrpc_monitor_io_handler_t *hd);
#endif /* USE_MINITR */
#endif /* _OMRPC_MON_H_ */


