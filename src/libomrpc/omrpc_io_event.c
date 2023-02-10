static char rcsid[] = "$Id: omrpc_io_event.c,v 1.1.1.1 2004-11-03 21:01:15 yoshihiro Exp $";
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
#define DEBUG_MXIO
/*
 * multi-threaded multiplex I/O
 */
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_rpc.h"

#ifdef USE_GLOBUS
#include "omrpc_globus.h"
#endif

static pthread_mutex_t io_lock;	/* general lock for omrpc_rpc */
#define IO_LOCK		pthread_mutex_lock(&io_lock)
#define IO_UNLOCK  	pthread_mutex_unlock(&io_lock)

static int to_fd,from_fd;
static int modified_flag;

fd_set rfds;
omrpc_io_port_t *current_pp;

static void *omrpc_handler_main(void *dummy);
int omrpc_handler_running;

/* prototype */
void omrpc_io_event_init()
{
    pthread_attr_t attr;
    pthread_t tid;
    int fds[2];
    int r;
    omrpc_io_port_t *pp;

    if(pipe(fds) != 0){
        perror("pipe");
        omrpc_fatal("cannot create pipe");
    }
    /* communication pipe between caller to handler */
    to_fd = fds[1];
    from_fd = fds[0];

    pthread_mutex_init(&io_lock,NULL);

    /* all mxio ports are activeated */
    for(pp = omrpc_port_head; pp != NULL; pp = pp->next){
      if(pp->port_type == PORT_MXIO){
          omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)pp;
          mport->active_flag = TRUE;
      }
    }

    /* create thread */
    pthread_attr_init(&attr);
    r = pthread_create(&tid, &attr, omrpc_handler_main, NULL);
    if(r != 0){
        omrpc_fatal("cannot create handler thread");
    }
    omrpc_handler_running = TRUE;

    if(omrpc_debug_flag) omrpc_prf("omrpc_io_event_init end ...\n");
}

void omrpc_io_lock() {  IO_LOCK; }
void omrpc_io_unlock() { IO_UNLOCK; }

omrpc_io_handle_t *omrpc_io_handle_create(omrpc_io_handle_t *hp)
{
    omrpc_io_port_t *port;
    port = hp->port;
    switch(port->port_type){
    case PORT_SINGLE:
    {
        int fd;
        unsigned short port = 0;

        IO_LOCK;
        fd = omrpc_io_socket(&port);
        hp = omrpc_io_handle_fd(fd,FALSE);
        hp->port_n = port;
        IO_UNLOCK;

        omrpc_io_modified();
        return hp;
    }
    case PORT_MXIO:
    {
        int i;
        omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)port;

        IO_LOCK;
        for(i = 1; i < MAX_HANDLE_PER_PORT; i++){
            if(mport->handles[i] == NULL) break;
        }
        if(i == MAX_HANDLE_PER_PORT)
            omrpc_fatal("too many mxio port");
        hp = omrpc_malloc(sizeof(omrpc_io_handle_t));
        hp->port = port;
        hp->port_n = i;
        mport->handles[i] = hp;
        IO_UNLOCK;

        if(omrpc_debug_flag) omrpc_prf("port mxio allocated=%d\n",i);

        return hp;
    }
#ifdef USE_GLOBUS
    case PORT_GLOBUS:
    case PORT_GLOBUS_MXIO:
        return omrpc_globus_io_handle_create(hp);
#endif
    default:
        omrpc_fatal("omrpc_create_io: bad port type %d",port->port_type);
        return NULL;
    }
}

void omrpc_io_modified()
{
    char c;

    if(!omrpc_handler_running) return;

#ifdef DEBUG_MXIO
    if(omrpc_debug_flag) omrpc_prf("io modified!!\n");
#endif
    modified_flag = TRUE;
    /* nofity restart server's select */
    if(write(to_fd,&c,1) != 1){
        perror("write");
        omrpc_fatal("write notify is failed");
    }
}

/*
 * handler thread main
 */
static void *omrpc_handler_main(void *dummy)
{
    pid_t pid;
    int status;
    omrpc_io_handle_t *hp;
    omrpc_rpc_t *rp;
    omrpc_request_t *req;

 again:
    while(omrpc_wait_any_input() == 0){
        /* check zombi */
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
                printf("pid %d is dead\n",pid);
        }
    }
    while((hp = omrpc_get_input_handle()) != NULL){
        if((rp = (omrpc_rpc_t *)hp->hint)  == NULL) continue;
        if((req = rp->req) == NULL) continue;
        omrpc_rpc_handler(req);
    }
    sched_yield();
    goto again;	/* loop forever */
}

/*
 * wait input event: called from handler thread
 */
int omrpc_wait_any_input()
{
    int max_nfd,fd,nfd;
    char c;
    struct timeval tm;
    omrpc_io_port_t *pp;

reset:
    IO_LOCK;
    modified_flag = FALSE;
    current_pp = omrpc_port_head;

    /* collect fds */
    FD_ZERO(&rfds);
    max_nfd = from_fd;
    FD_SET(from_fd,&rfds);
    for(pp = omrpc_port_head; pp != NULL; pp = pp->next){
        switch(pp->port_type){
        case PORT_SINGLE:
            fd = pp->fd;
            break;
        case PORT_MXIO:
            {
                omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)pp;
                if(mport->active_flag) fd = mport->fd;
                else fd = 0;
                break;
            }
        case PORT_GLOBUS:
        case PORT_GLOBUS_MXIO:
        default:
            fd = 0;
        }

        if(fd <= 0) continue;
        if(fd > max_nfd) max_nfd = fd;
        FD_SET(fd,&rfds);
    }
    IO_UNLOCK;
#ifdef DEBUG_MXIO
    if(omrpc_debug_flag)
        omrpc_prf("omrpc_handler do select max_nfd=%d ...\n",max_nfd);
#endif

retry:
    tm.tv_sec = TIMEOUT_SEC;
    tm.tv_usec = 0;
    nfd = select(max_nfd+1,&rfds,NULL,NULL,&tm);
#ifdef DEBUG_MXIO
    if(omrpc_debug_flag) omrpc_prf("select is done nfd=%d\n",nfd);
#endif
    if(nfd < 0){
        if (errno == EINTR) {
            goto retry;
        }
        perror("select");
        omrpc_fatal("handler select failed");
    }

    if(FD_ISSET(from_fd,&rfds)){
        if(read(from_fd,&c,1) != 1){
            perror("read");
            omrpc_fatal("read notify is failed");
        }
        goto reset;
    }
    return nfd;
}

omrpc_io_handle_t *omrpc_get_input_handle()
{
    int fd;
    omrpc_io_port_t *port;
    omrpc_io_handle_t *hp;

    IO_LOCK;
    if(modified_flag){
        IO_UNLOCK;
        return NULL;
    }

    while(1){
        if(current_pp == NULL){
            port = NULL;
            break;
        }
        port = current_pp;
        current_pp = current_pp->next;
        fd = port->fd;
        if(fd < 0) omrpc_fatal("omrpc_get_input_io_handle: fd < 0");
        if(FD_ISSET(fd,&rfds)) break;
    }
    IO_UNLOCK;

#ifdef DEBUG_MXIO
    if(omrpc_debug_flag)
        omrpc_prf("get_input_handle port=0x%x...\n",port);
#endif

    if(port == NULL) return NULL;

    switch(port->port_type){
    case PORT_SINGLE:
        if((hp = port->hp) == NULL)
            omrpc_fatal("omrpc_get_input_handle: port's io_handle is null");
        return hp;
    case PORT_MXIO:
        {
            omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)port;
            int port_n,nbytes;

            pthread_mutex_lock(&mport->lock);
            if(!mport->active_flag){	/* not active, don't read */
                pthread_mutex_unlock(&mport->lock);
                return NULL;
            }

            /* peek input */
            omrpc_read_mxio_header(mport,&port_n,&nbytes);
            if((hp = mport->handles[port_n]) == NULL)
                omrpc_fatal("get_input_handle: mport's io_handle is null");
            if(hp->ncount != hp->pos)
                omrpc_fatal("get_input_handle: handle buffer not empty");
            if(nbytes != omrpc_read_nbytes(mport->fd,hp->buf,nbytes))
                omrpc_fatal("get_input_handle: mxio error");
            hp->peeked_ncount = nbytes;
            hp->peeked = TRUE;
#if 0
            mport->active_flag = FALSE;  /* already take input, de-activate */
#endif
            if(omrpc_debug_flag)
                omrpc_prf("peek input nport_n=%d,nbytes=%d\n",port_n,nbytes);

            if(port_n == 0){ /* mgr port (port_n) is special */
                pthread_cond_signal(&mport->cond);
                hp =  NULL;
            }
            pthread_mutex_unlock(&mport->lock);

            return hp;
        }
    case PORT_GLOBUS:
    case PORT_GLOBUS_MXIO:
    default:
        omrpc_fatal("omrpc_get_input_handle: bad port type %d",
                    port->port_type);
        return NULL;
    }
}




