static char rcsid[] = "$Id: omrpc_agent_globus.c,v 1.1.1.1 2004-11-03 21:01:19 yoshihiro Exp $";

#ifdef USE_GLOBUS

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
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_globus.h"
#include "omrpc_agent_defs.h"

/*
 * for support for I/O multiplex
 */
static fd_set rfds;
static int to_fd,from_fd;

static char buf[OMRPC_IO_BUF_SIZE];

static pthread_mutex_t agent_lock;
#define AGENT_LOCK        pthread_mutex_lock(&agent_lock)
#define AGENT_UNLOCK      pthread_mutex_unlock(&agent_lock)

static void *omrpc_globus_agent_handler_main(void *dummy);

static omrpc_globus_mxio_port_t *my_port;

void omrpc_agent_globus_mxio_init(omrpc_io_port_t *port)
{
    static pthread_attr_t pth_attr;
    static pthread_t tid;
    int fds[2];
    int r;

    if(port->port_type != PORT_GLOBUS_MXIO)
        omrpc_fatal("bad port type in gloubs agent mxio_init type=%d",
                    port->port_type);

    my_port = (omrpc_globus_mxio_port_t *)port;

    if(pipe(fds) != 0){
        perror("pipe");
        omrpc_fatal("cannot create pipe");
    }
    /* communication pipe between caller to handler for wakeup */
    to_fd = fds[1];
    from_fd = fds[0];

    pthread_mutex_init(&agent_lock,NULL);
    AGENT_LOCK;        /* lock first */

    pthread_attr_init(&pth_attr);
    r = pthread_create(&tid, &pth_attr, omrpc_globus_agent_handler_main, NULL);
    if(r != 0){
        omrpc_fatal("cannot create globus handler thread");
    }

    omrpc_agent_narrow_mode = FALSE;
}

static char *io_result_string(globus_result_t result);

/* waiting input */
void omrpc_agent_globus_mxio_recv_check()
{
    globus_result_t result;
    int r;
    short hbuf[2];
    int port_n,nbytes;
    omrpc_io_handle_t *hp;
    omrpc_rex_proc_t *rp;

    if(omrpc_debug_flag)
        omrpc_prf("globus_mxio_recv_check ...\n");
again:
    AGENT_UNLOCK;
    /* reading packet header */
    result = globus_io_read(&my_port->g_handle,(char *)hbuf,
                            sizeof(short)*2, sizeof(short)*2,&r);
    AGENT_LOCK;

    if (result != GLOBUS_SUCCESS ){
        globus_object_t *  err = globus_error_get(result);
        if(!globus_io_eof(err)){
            omrpc_prf("error: %s %s\n",
                      globus_object_printable_to_string(err),
                      io_result_string(result));
            omrpc_fatal("header read error in globus_mxio_recv_check");
        }
        /* EOF */
        hp = my_port->handles[0];
        hp->peeked = TRUE;
        hp->peeked_ncount = 0;
        return;
    }

    port_n = hbuf[0];
    nbytes = hbuf[1];
    if(port_n == 0){
        hp = my_port->handles[0];
        hp->peeked = TRUE;
        hp->peeked_ncount = nbytes;
        result = globus_io_read(&my_port->g_handle,hp->buf,nbytes,nbytes,&r);
        if (result != GLOBUS_SUCCESS || r != nbytes)
            omrpc_fatal("read error in globus_mxio_recv_check");
        omrpc_agent_narrow_mode = TRUE;
        omrpc_agent_narrow_port_n = 0;
        return;
    } else {
        /* read body and forward it */
        result = globus_io_read(&my_port->g_handle,buf,nbytes,nbytes,&r);
        if (result != GLOBUS_SUCCESS || r != nbytes)
            omrpc_fatal("read error in globus_mxio_recv_check");

        /* forward */
        if((rp = omrpc_agent_procs[port_n]) == NULL)
            omrpc_fatal("no proc in globus_mxio_recv_check");
        r = omrpc_write_nbytes(rp->fd,buf,nbytes);
        if(r != nbytes)  omrpc_fatal("write error to rex prog");
        goto again;
    }
}

/*
 * check I/O from rex, and forward to client.
 */
static void *omrpc_globus_agent_handler_main(void *dummy)
{
    int i,max_nfd,nfd,r;
    int max_port_n;
    int nbytes,port_n;
    omrpc_rex_proc_t *rp;
    struct timeval tm;
    char c;
    globus_result_t result;

next:
    max_port_n = 0;
    if(!omrpc_agent_narrow_mode){
        for(i = 0; i < MAX_HANDLE_PER_PORT; i++){
            if(omrpc_agent_procs[i] != NULL)
                max_port_n = i+1;
        }
    }

#ifdef DEBUG
    if(omrpc_debug_flag)
        omrpc_prf("mxio_recv_check max_port_n=%d, narrow_mode=%d(%d)\n",
                  max_port_n,omrpc_agent_narrow_mode,omrpc_agent_narrow_port_n);
#endif

    /* do select */
    FD_ZERO(&rfds);

    /* from wakeup */
    max_nfd = from_fd;
    FD_SET(from_fd,&rfds);
    if(omrpc_agent_narrow_mode){
        if(omrpc_agent_narrow_port_n != 0){
            rp = omrpc_agent_procs[omrpc_agent_narrow_port_n];
            if(rp->fd > max_nfd) max_nfd = rp->fd;
            FD_SET(rp->fd,&rfds);
        }
    } else {    /* wide case */
        for(i = 0; i < max_port_n; i++){
            if((rp = omrpc_agent_procs[i]) == NULL) continue;
            if(rp->fd > max_nfd) max_nfd = rp->fd;
            FD_SET(rp->fd,&rfds);
        }
    }

    /* omrpc_prf("recv_check: select max_nfd=%d\n",max_nfd); */
 retry_select:
    AGENT_UNLOCK;
    tm.tv_sec = OMRPC_TIMEOUT_SEC;
    tm.tv_usec = 0;
    nfd = select(max_nfd+1,&rfds,NULL,NULL,&tm);
    AGENT_LOCK;

    if(nfd < 0){
        if (errno == EINTR) goto retry_select;
        perror("select");
        omrpc_fatal("handler select failed");
    }

    if(omrpc_debug_flag)
        omrpc_prf("recv_check: select done nfd=%d mode=%d\n",
                  nfd,omrpc_agent_narrow_mode);

    if(nfd == 0) goto next;

    if(omrpc_debug_flag)
        omrpc_prf("check packet from rex, narrow_mode=%d, port=%d\n",
                  omrpc_agent_narrow_mode,omrpc_agent_narrow_port_n);

    if(FD_ISSET(from_fd,&rfds)){
        if(read(from_fd,&c,1) != 1){
            perror("read");
            omrpc_fatal("read notify is failed");
        }
        if(omrpc_debug_flag) omrpc_prf("input from wakeup...\n");
        goto next;
    }

    if(omrpc_agent_narrow_mode){
        if(omrpc_agent_narrow_port_n == 0) goto next;
        if((rp = omrpc_agent_procs[omrpc_agent_narrow_port_n]) == NULL)
            omrpc_fatal("no proc in narrow mode");
        if(!FD_ISSET(rp->fd,&rfds)){
            if(omrpc_debug_flag) omrpc_prf("narrow mode, input from other...\n");
            goto next;
        }
    } else {
        omrpc_agent_narrow_port_n++;
        for(i = 0; i < max_port_n; i++,omrpc_agent_narrow_port_n++){
            if(omrpc_agent_narrow_port_n >= max_port_n)
                omrpc_agent_narrow_port_n = 0;
            /* comming packet from rex */
            if((rp = omrpc_agent_procs[omrpc_agent_narrow_port_n]) == NULL) continue;
            if(FD_ISSET(rp->fd,&rfds)) break;
        }
        /* if no port have input, go to top */
        if(i == max_port_n){
            if(omrpc_debug_flag) omrpc_prf("input port not found...\n");
            goto next;
        }
        /* turn on narrow_mode */
        omrpc_agent_narrow_mode = TRUE;
        if(omrpc_debug_flag)
            omrpc_prf("recv_check: narrow mode start, port=%d\n",
                      omrpc_agent_narrow_port_n);
    }

    if(omrpc_debug_flag)
        omrpc_prf("recv_check:input from rex, nport=%d\n",
                  omrpc_agent_narrow_port_n);

    /* read from narrow port, and send it to client in packet */
retry_read:
    nbytes = read(rp->fd,buf,OMRPC_IO_BUF_SIZE);
    if(nbytes < 0){
        if (errno == EINTR) goto retry_read;
        omrpc_fatal("read error from rex");
    }

    if(omrpc_debug_flag)
        omrpc_prf("recv_check:input from rex, done nbytes=%d\n",nbytes);

    if(nbytes == 0) omrpc_fatal("read error form rex: unexpected EOF");
    omrpc_globus_write_mxio_header(my_port,omrpc_agent_narrow_port_n,nbytes);
    /* send body */
    result = globus_io_write(&my_port->g_handle,buf,nbytes,&r);
    if (result != GLOBUS_SUCCESS || r != nbytes)
        omrpc_fatal("error in globus_io_write");

    if(omrpc_debug_flag)
        omrpc_prf("send to client, nbytes=%d\n",nbytes);
    goto next;
}

void omrpc_agent_globus_mxio_wakeup()
{
    char c;

    if(omrpc_debug_flag) omrpc_prf("agent_globus_mxio_wakeup...\n");
    if(write(to_fd,&c,1) != 1){
        perror("write");
        omrpc_fatal("write notify is failed");
    }
}


static char *io_result_string(globus_result_t result)
{
    if(result == GLOBUS_IO_ERROR_TYPE_NULL_PARAMETER)
        return "GLOBUS_IO_ERROR_TYPE_NULL_PARAMETER";
    if(result == GLOBUS_IO_ERROR_TYPE_NOT_INITIALIZED)
        return "GLOBUS_IO_ERROR_TYPE_NOT_INITIALIZED";
    if(result == GLOBUS_IO_ERROR_TYPE_CLOSE_ALREADY_REGISTERED)
        return "GLOBUS_IO_ERROR_TYPE_CLOSE_ALREADY_REGISTERED";
    if(result == GLOBUS_IO_ERROR_TYPE_READ_ALREADY_REGISTERED)
        return "GLOBUS_IO_ERROR_TYPE_READ_ALREADY_REGISTERED";
    if(result == GLOBUS_ERROR_TYPE_TYPE_MISMATCH)
        return "GLOBUS_ERROR_TYPE_TYPE_MISMATCH";
    return "GLOBUS_IO_ERROR_TYPE_????";
}

#endif
