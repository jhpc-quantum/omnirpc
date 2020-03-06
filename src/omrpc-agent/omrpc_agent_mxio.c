static char rcsid[] = "$Id: omrpc_agent_mxio.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $";
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

#include "omrpc_agent_defs.h"

/*
 * for support for I/O multiplex
 */
static fd_set rfds,wfds;

int omrpc_agent_narrow_mode; /* true if see all port */
int omrpc_agent_narrow_port_n;

#ifdef INSPECT_PACKET
static void dumpBuffer(char *buf, int size, char* str){
    int i;

    omrpc_prf("DumpBuffer (%s:%s:%p) start size = %d",
              omrpc_is_client ? "client" : "stub",str,buf, size);
    for(i = 0; i < size; i++){
        if( i % 20 == 0){
            fprintf(stdout, "\n");
            omrpc_prf("BUF(%s:%s:%p): ",
                      omrpc_is_client ? "client" : "stub",str,buf);
        }
        if( (i % 4) == 0){
            fprintf(stdout, " ");
        }
        fprintf(stdout, "%02X", buf[i]);
    }

    fprintf(stdout, "\n");
    omrpc_prf("DumpBuffer (%s:%s:%p) end \n",
              omrpc_is_client ? "client" : "stub",str,buf);
    fflush(stdout);
}
#endif
static char buf[OMRPC_IO_BUF_SIZE];
omrpc_mxio_port_t *my_port;

void omrpc_agent_mxio_init(omrpc_io_port_t *port)
{
    if(port->port_type != PORT_MXIO)
        omrpc_fatal("bad port type in agent mxio_init type=%d",
                    port->port_type);

    my_port = (omrpc_mxio_port_t *)port;

    omrpc_agent_narrow_mode = FALSE;
}

void omrpc_agent_mxio_recv_check()
{
    int i,max_nfd,nfd;
    int max_port_n;
    int nbytes,port_n;
    omrpc_rex_proc_t *rp = NULL;
    struct timeval tm;

 restart:
    max_port_n = 0;
    if(!omrpc_agent_narrow_mode){
        for(i = 0; i < MAX_HANDLE_PER_PORT; i++){
            if(omrpc_agent_procs[i] != NULL)
                max_port_n = i+1;
        }
    }

    if(omrpc_debug_flag)
        omrpc_prf("mxio_recv_check max_port_n=%d, narrow_mode=%d(%d)\n",
                  max_port_n,omrpc_agent_narrow_mode,omrpc_agent_narrow_port_n);

 next:
    /* do select */
    FD_ZERO(&rfds);

    /* from client */
    max_nfd = my_port->fd;
    FD_SET(my_port->fd,&rfds);
    if(omrpc_agent_narrow_mode){
        rp = omrpc_agent_procs[omrpc_agent_narrow_port_n];
        if(rp->fd > max_nfd) max_nfd = rp->fd;
        FD_SET(rp->fd,&rfds);
    } else {        /* wide case */
        for(i = 0; i < max_port_n; i++){
            if((rp = omrpc_agent_procs[i]) == NULL) continue;
            if(rp->fd > max_nfd) max_nfd = rp->fd;
            FD_SET(rp->fd,&rfds);
        }
    }

    /* omrpc_prf("recv_check: select max_nfd=%d\n",max_nfd); */
 retry_select:
    tm.tv_sec = OMRPC_TIMEOUT_SEC_DEFAULT;
    tm.tv_usec = 0;
    nfd = select(max_nfd+1,&rfds,NULL,NULL,&tm);

    if(nfd < 0){
        if (errno == EINTR) goto retry_select;
        perror("select");
        omrpc_fatal("handler select failed");
    }

    /* omrpc_prf("recv_check: select done nfd=%d\n",nfd); */

    if(nfd == 0) goto next;

    if(FD_ISSET(my_port->fd,&rfds)){        /* from client */
        omrpc_read_mxio_header(my_port,&port_n,&nbytes);

        if(omrpc_debug_flag)
            omrpc_prf("recv_check: input from client, port_n=%d, nbytes=%d\n",
                      port_n,nbytes);

        if(port_n == 0 || nbytes == 0){
            omrpc_io_handle_t *hp = my_port->handles[0];
            if(hp->ncount != hp->pos)
                omrpc_fatal("handle buffer not empty");
            if(nbytes != 0 &&
               nbytes != omrpc_read_nbytes(my_port->fd,hp->buf,nbytes))
                omrpc_fatal("read error from client");
            if(omrpc_debug_flag)
                omrpc_prf("check packet, peek 0 nbytes=%d\n",nbytes);
#ifdef INSPECT_PACKET
            //debug
            dumpBuffer(hp->buf, nbytes,"client->agent");
#endif
            hp->peeked = TRUE;
            hp->peeked_ncount = nbytes;
            return;       /* send to manger */
        }

        if((rp = omrpc_agent_procs[port_n]) == NULL)
            omrpc_fatal("packet to non-allocated proc");

        if(nbytes != omrpc_read_nbytes(my_port->fd,buf,nbytes))
            omrpc_fatal("read error from client");
        if(nbytes != omrpc_write_nbytes(rp->fd,buf,nbytes))
            omrpc_fatal("write error to rex");
#ifdef INSPECT_PACKET
        //debug
        {
            char bbbuf[100];
            snprintf(bbbuf,sizeof(bbbuf), "client->rex[%d]",port_n);
            dumpBuffer(buf, nbytes,bbbuf);
        }
#endif
    }

    /* check for write to client */
    FD_ZERO(&wfds);
    max_nfd = my_port->fd;
    FD_SET(my_port->fd,&wfds);
    tm.tv_sec = 0;
    tm.tv_usec = 0;
    nfd = select(max_nfd+1,NULL,&wfds,NULL,&tm);
    if(nfd <= 0 || !FD_ISSET(my_port->fd,&wfds)){
      if(omrpc_debug_flag) omrpc_prf("write client is not ready\n");
      goto next;
    }

    if(omrpc_debug_flag)
        omrpc_prf("check packet from rex, narrow_mode=%d, port=%d\n",
                  omrpc_agent_narrow_mode,omrpc_agent_narrow_port_n);

    if(omrpc_agent_narrow_mode){
        if((rp = omrpc_agent_procs[omrpc_agent_narrow_port_n]) == NULL)
            omrpc_fatal("no proc in narrow mode");
        if(!FD_ISSET(rp->fd,&rfds)) goto next;
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
            if(omrpc_debug_flag)
                omrpc_prf("recv_check: no input from rex\n");
            goto next;
        }
        /* turn on omrpc_agent_narrow_mode */
        omrpc_agent_narrow_mode = TRUE;

        if(omrpc_debug_flag)
            omrpc_prf("recv_check: narrow mode start, port=%d\n",
                      omrpc_agent_narrow_port_n);

    }

    if(omrpc_debug_flag)
        omrpc_prf("recv_check:input from rex, port=%d\n",
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

    if(nbytes == 0){
        /* omrpc_fatal("read error form rex: unexpected EOF"); */
        omrpc_agent_narrow_mode = FALSE;
        omrpc_agent_procs[omrpc_agent_narrow_port_n] = NULL;
        goto restart;
    }
#ifdef INSPECT_PACKET
    //debug
    {
        char bbbuf[100];
        snprintf(bbbuf,sizeof(bbbuf), "rex[%d]->client",omrpc_agent_narrow_port_n);
        dumpBuffer(buf, nbytes,bbbuf);
    }
#endif
    omrpc_write_mxio_header(my_port,omrpc_agent_narrow_port_n,nbytes);
    omrpc_write_nbytes(my_port->fd,buf,nbytes);
    goto next;
}

omrpc_rex_proc_t *
omrpc_agent_mxio_submit(char *path, int port_num)
{
    int fd;
    omrpc_rex_proc_t *rp;
    unsigned short port;

    port = 0;
    fd = omrpc_io_socket(&port);
    rp = omrpc_agent_submit(path,NULL,port,1);
    if(rp == NULL) return NULL;

    fd = omrpc_io_accept(fd);
    rp->fd = fd;

    if(port_num == 0) omrpc_fatal("agent_mxio_submit, port_num == 0");
    omrpc_agent_procs[port_num] = rp;

    if(omrpc_debug_flag)
        omrpc_prf("mxio_submit port_num=%d, port=%d, fd=%d\n",
                  port_num,port,fd);

    return rp;
}

void omrpc_agent_mxio_recv_widen()
{
    omrpc_agent_narrow_mode = FALSE;
}


