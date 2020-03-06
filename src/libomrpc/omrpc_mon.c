/* 
 * $Id: omrpc_mon.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $
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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include "omrpc_defs.h"
#include "omrpc_mon.h"
#include "omrpc_stub.h"

#ifdef USE_MONITOR
static pthread_mutex_t monitor_lock;  /* general lock for manger task */
#define MONITOR_LOCK        pthread_mutex_lock(&monitor_lock)
#define MONITOR_UNLOCK      pthread_mutex_unlock(&monitor_lock)
int omrpc_monitor_interval = 2;
omrpc_monitor_io_handler_t *omrpc_mon_handl;

void
omrpc_monitor_init()
{
    int fd;

    pthread_mutex_init(&monitor_lock,NULL);
    MONITOR_LOCK;

    omrpc_mon_handl =
        (omrpc_monitor_io_handler_t *)omrpc_malloc(sizeof(omrpc_monitor_io_handler_t));

    memset(&(omrpc_mon_handl->sin), 0, sizeof(omrpc_mon_handl->sin));
    omrpc_mon_handl->sin.sin_family = AF_INET;
    omrpc_mon_handl->sin.sin_port = htons(omrpc_monitor_port);

    if(omrpc_debug_flag)
        omrpc_prf("connect host=%s:%d\n",
                omrpc_client_hostname,omrpc_monitor_port);

    omrpc_mon_handl->hp = gethostbyname(omrpc_client_hostname);
    if(omrpc_mon_handl->hp == NULL){
        perror("gethostbyname");
        omrpc_prf("gethostname host='%s'", omrpc_client_hostname);
        exit(1);
    }

    bcopy(omrpc_mon_handl->hp->h_addr, &omrpc_mon_handl->sin.sin_addr.s_addr,
          omrpc_mon_handl->hp->h_length);

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0){
        perror("socket failed");
        omrpc_prf("socket");
        exit(1);
    }
    omrpc_mon_handl->fd = fd;

    MONITOR_UNLOCK;
}


void *
omrpc_monitor_handler(void *dummy)
{
    while(1){
        omrpc_monitor_info_send_by_load(omrpc_mon_handl);
        MONITOR_LOCK;
        sleep(omrpc_monitor_interval);
        MONITOR_UNLOCK;
    }
}


int
omrpc_monitor_info_send_by_load(omrpc_monitor_io_handler_t *hd)
{
    double load[3];
    char msg[2048];

    if(getloadavg(load, 3) < 3)
        omrpc_prf("cannot get load average");
    if(omrpc_debug_flag)
        omrpc_prf("%s:%0.2f %0.2f %0.2f\n",
                  omrpc_my_hostname, load[0], load[1], load[2]);

    snprintf(msg, sizeof(msg),"%s\n%4.2f\n", omrpc_my_hostname, load[0]);
    if(omrpc_monitor_io_send_msg(hd, msg) < strlen(msg)){
        omrpc_prf("cannot send message correctly");
        return -1;
    }

    return 0;
}

int
omrpc_monitor_io_send_msg(omrpc_monitor_io_handler_t *hd, char *msg)
{
    return sendto(hd->fd, msg, strlen(msg), 0,
                  (const struct sockaddr*)&(hd->sin), sizeof(hd->sin));
}
#endif /* USE_MONITOR */
