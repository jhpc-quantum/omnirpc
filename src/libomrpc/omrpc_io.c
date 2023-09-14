
static char rcsid[] = "$Id: omrpc_io.c,v 1.2 2006-01-25 16:06:17 ynaka Exp $";
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
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_mon.h"

#ifdef USE_GLOBUS
#include "omrpc_globus.h"
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 1024
#endif

int omrpc_is_client = FALSE;
int omrpc_monitor_flag = FALSE;

char *omrpc_my_hostname = "???";
omrpc_io_port_t *omrpc_port_head,*omrpc_port_tail;

char *omrpc_monitor_target_host = NULL;
int omrpc_monitor_port = OMRPC_MONITOR_DEFAULT_PORT;

int omrpc_timeout_sec = OMRPC_TIMEOUT_SEC_DEFAULT;

/* test */
static void dumpBuffer(char *, int, char *);

/* prototype */
static omrpc_io_port_t *omrpc_new_port(int size,char type);

void omrpc_io_init(void)
{
    int r;
    char *me;
    char hostname[MAXHOSTNAMELEN];
    struct hostent *hp;

    if((me = getenv("OMRPC_HOSTNAME")) == NULL){
      /* get client host name */
      r = gethostname(hostname,MAXHOSTNAMELEN);
      if(r < 0){
          perror("hostname");
          exit(1);
      }

      /* check FQDN and use it if any */
      hp = gethostbyname(hostname);
      if (hp != NULL && hp->h_name != NULL && *(hp->h_name) != '\0') {
        omrpc_my_hostname = strdup(hp->h_name);
      } else {
        omrpc_my_hostname = strdup(hostname);
      }
    } else {
      omrpc_my_hostname = strdup(me);
    }

    /* initialize variable */
    omrpc_port_head = NULL;
}

void omrpc_io_finalize()
{

}

static omrpc_io_port_t *omrpc_new_port(int size,char type)
{
    omrpc_io_port_t *pp;

    pp = (omrpc_io_port_t *)omrpc_malloc(size);
    pp->port_type = type;

    /* put it on the list */
    if(omrpc_port_head == NULL){
        omrpc_port_head = omrpc_port_tail = pp;
    } else {
        /* put tail */
        omrpc_port_tail->next = pp;
        pp->prev = omrpc_port_tail;
        omrpc_port_tail = pp;
    }
    return pp;
}

void omrpc_delete_port(omrpc_io_port_t *port)
{
    if(port == omrpc_port_head){
        omrpc_port_head = port->next;
        if(omrpc_port_head != NULL) omrpc_port_head->prev = NULL;
    } else if(port == omrpc_port_tail){
        omrpc_port_tail = port->prev;
        port->prev->next = NULL;
    } else {
        port->next->prev = port->prev;
        port->prev->next = port->next;
    }
}

/* create initial I/O handle for fd */
omrpc_io_handle_t *omrpc_io_handle_fd(int fd, int mxio_flag)
{
    omrpc_io_handle_t *hp;

    if(!mxio_flag){
        omrpc_io_port_t *pp;
        pp = omrpc_new_port(sizeof(omrpc_io_port_t),PORT_SINGLE);
        pp->fd = fd;
        hp = omrpc_malloc(sizeof(omrpc_io_handle_t));
        hp->port = pp;
        pp->hp = hp;
    } else {
        omrpc_mxio_port_t *pp;

        pp = (omrpc_mxio_port_t *)
            omrpc_new_port(sizeof(omrpc_mxio_port_t),PORT_MXIO);

        pthread_mutex_init(&pp->lock,NULL);
        pthread_cond_init(&pp->cond,NULL);
        pp->fd = fd;
        hp = omrpc_malloc(sizeof(omrpc_io_handle_t));
        hp->port = (omrpc_io_port_t *)pp;
        hp->port_n = 0;

        pp->handles[0] = hp;   /*  zero for agent */
    }
    return hp;
}


/*
 * flush and fill
 */
void omrpc_io_handle_fill(omrpc_io_handle_t *hp)
{
    omrpc_io_port_t *port = hp->port;
    int r;

    if(hp->peeked){     /* if peeked */
        hp->peeked = FALSE;
        hp->pos = 0;
        hp->ncount = hp->peeked_ncount;
        return;
    }
    switch(port->port_type){
    case PORT_SINGLE:
    again:
        r = read(port->fd,hp->buf,OMRPC_IO_BUF_SIZE);
        if(r==0) perror("read");
#ifdef INSPECT_PACKET
        // debug
        dumpBuffer(hp->buf, r, "fill");
#endif
        if(r < 0){
            if(errno == EINTR) goto again;
            perror("fill");
            goto err;
        }
        hp->ncount = r;
        hp->pos = 0;

        return;

    case PORT_MXIO:
    {
        int port_n,nbytes;
        omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)port;

        pthread_mutex_lock(&mport->lock);

        if(mport->active_flag && hp->port_n == 0){
            /* port 0 is read by other thread */
            while(!hp->peeked)
                pthread_cond_wait(&mport->cond,&mport->lock);
            hp->peeked = FALSE;
            hp->pos = 0;
            hp->ncount = hp->peeked_ncount;
            pthread_mutex_unlock(&mport->lock);
            return;
        }

        /* the first packet must be already read by io_event_handler */
    next:
        /* read header */
        omrpc_read_mxio_header(mport,&port_n,&nbytes);
        if(port_n != hp->port_n){
            if(port_n == 0){
                omrpc_io_handle_t *hp0 = mport->handles[0];
                if(hp0->peeked)
                    omrpc_fatal("peeked port is already filled");
                if(nbytes != omrpc_read_nbytes(mport->fd,hp0->buf,nbytes))
                    omrpc_fatal("get_input_handle: mxio error");
                hp0->peeked_ncount = nbytes;
                hp0->peeked = TRUE;
                /* send signal to port */
                pthread_cond_signal(&mport->cond);
                goto next;
            } else{
                omrpc_fatal("invalid message in MXIO, %d != %d (fill)",
                            port_n, hp->port_n);
            }
        }
#ifdef DEBUG_MXIO
        if(omrpc_debug_flag)
            omrpc_prf("fill mxio port=%d,nbyte=%d\n",port_n,nbytes);
#endif
        hp->ncount = nbytes;
#ifdef INSPECT_PACKET
        //debug
        dumpBuffer(hp->buf, nbytes, "fill(mxio)");
#endif
        if(nbytes != 0){
            if(nbytes != omrpc_read_nbytes(mport->fd,hp->buf,nbytes))
                goto err;
        }
        hp->pos = 0;

        pthread_mutex_unlock(&mport->lock);

        return;
    }

#ifdef USE_GLOBUS
    case PORT_GLOBUS:
    case PORT_GLOBUS_MXIO:
        omrpc_globus_io_handle_fill(hp);
        return;
#endif
    default:
        omrpc_fatal("omrpc_io_fill: bad port type %d",port->port_type);
    }
 err:
    omrpc_fatal("omrpc_io_fill: IO error");
}

void omrpc_io_handle_flush(omrpc_io_handle_t *hp)
{
    omrpc_io_port_t *port = hp->port;

    switch(port->port_type){
    case PORT_SINGLE:
#if 0
        //debug
        dumpBuffer(hp->buf, hp->pos, "flush");
#endif
        if(hp->pos != omrpc_write_nbytes(port->fd,hp->buf,hp->pos))
            goto err;
        hp->pos = 0;
        return;
    case PORT_MXIO:
    {
        omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)port;

        if(omrpc_debug_flag)
            omrpc_prf("flush mxio port=%d,nbyte=%d\n", hp->port_n,hp->pos);

        omrpc_io_lock();
        omrpc_write_mxio_header(mport,hp->port_n,hp->pos);
#ifdef INSPECT_PACKET
        //debug
        dumpBuffer(hp->buf, hp->pos ,"flush(mxio)");
#endif
        if(hp->pos != omrpc_write_nbytes(mport->fd,hp->buf,hp->pos))
            goto err;
        hp->pos = 0;
        omrpc_io_unlock();
        return;
    }
#ifdef USE_GLOBUS
    case PORT_GLOBUS:
    case PORT_GLOBUS_MXIO:
        omrpc_globus_io_handle_flush(hp);
        return;
#endif
    default:
        omrpc_fatal("omrpc_io_flush: bad port type %d",port->port_type);
    }
err:
    omrpc_fatal("omrpc_io_flush: IO error");
}

void omrpc_send_done(omrpc_io_handle_t *p)
{
    omrpc_io_handle_flush(p);
    p->pos = 0;
    p->ncount = 0;
}

void omrpc_recv_done(omrpc_io_handle_t *p)
{
    /* rest count */
    p->pos = 0;
    p->ncount = 0;

    omrpc_recv_done_port(p->port);	/* for port specific operation */
}

void omrpc_recv_done0(omrpc_io_handle_t *p)
{
    /* rest count */
    p->pos = 0;
    p->ncount = 0;
}

void omrpc_read_mxio_header(omrpc_mxio_port_t *port,
                            int *port_n, int *nbytes)
{
    short b[2];
    int r;
    r = omrpc_read_nbytes(port->fd,(char *)b,sizeof(b));
    if(r == 0){  /* EOF */
        *port_n = 0;
        *nbytes = 0;
        return;
    }
    if(r != sizeof(b))
        omrpc_fatal("omrpc_write_mxio_header: IO error");

    if(port->swap_flag){
        omrpc_swap_short(&b[0],1);
        omrpc_swap_short(&b[1],1);
    }
    *port_n = b[0];
    *nbytes = b[1];
}

void omrpc_write_mxio_header(omrpc_mxio_port_t *port,
                             int port_n, int nbytes)
{
    short b[2];
    if(nbytes == 0) return;
    b[0] = port_n;
    b[1] = nbytes;
    if(sizeof(b) != omrpc_write_nbytes(port->fd,(char *)b,sizeof(b)))
        omrpc_fatal("omrpc_write_mxio_header: IO error");
}

void omrpc_send_mxio_ack(omrpc_mxio_port_t *port,char cmd)
{
    omrpc_io_lock();
    omrpc_write_mxio_header(port,0,1);
    omrpc_write_nbytes(port->fd,&cmd,1);
    omrpc_io_unlock();
}

int omrpc_read_nbytes(int fd, char *cp, int nbytes)
{
    int r,n;
    n = nbytes;
    while(n > 0){
        r = read(fd,cp,n);
        if(r <= 0){
            if(errno == EINTR) continue;
            /* perror("omrpc_read_nbytes:"); */
            return r;
        }
        n -= r;
        cp += r;
    }
    return nbytes;
}

int omrpc_write_nbytes(int fd, char *cp, int nbytes)
{
    int r,n;
    n = nbytes;
    while(n > 0){
        r = write(fd,cp,n);
        if(r <= 0){
            if(errno == EINTR) continue;
	     /* perror("omrpc_write_nbytes:"); */
            return r;
        }
        n -= r;
        cp += r;
    }
    return nbytes;
}

short omrpc_io_handle_port_n(omrpc_io_handle_t *hp)
{
    return hp->port_n;
}

void omrpc_io_handle_close(omrpc_io_handle_t *hp)
{
    omrpc_io_port_t *port = hp->port;

    switch(port->port_type){
    case PORT_SINGLE:
        close(port->fd);
        omrpc_delete_port(port);
        omrpc_free(port);
        omrpc_free(hp);
        return;

    case PORT_MXIO:
    {
        omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)port;
        mport->handles[hp->port_n] = NULL;
        omrpc_free(hp);
        return;
    }
#ifdef USE_GLOBUS
    case PORT_GLOBUS:
    case PORT_GLOBUS_MXIO:
        omrpc_globus_io_handle_close(hp);
        return;
#endif
    default:
        omrpc_fatal("omrpc_io_handle_close: bad port type %d",port->port_type);
    }
}

void omrpc_io_handle_accept(omrpc_io_handle_t *hp)
{
    omrpc_io_port_t *port = hp->port;

    switch(port->port_type){
    case PORT_SINGLE:
    {
        int fd;
        fd = port->fd;
        port->fd = omrpc_io_accept(fd);
        close(fd);
        return;
    }
    case PORT_MXIO:
        /* not accept action */
        return;

#ifdef USE_GLOBUS
    case PORT_GLOBUS:
    case PORT_GLOBUS_MXIO:
        /* not accept action */
        return;
#endif
    default:
        omrpc_fatal("omrpc_io_handle_accept: bad port type %d",
                    port->port_type);
    }
}

/*
 * initialize, set swap flag
 */
void omrpc_io_handle_byte_order(omrpc_io_handle_t *hp,int is_client)
{
    short s;

    if(omrpc_debug_flag) omrpc_prf("check byte order ...\n");

    switch(hp->port->port_type){
    case PORT_MXIO:
    {
        omrpc_mxio_port_t *mport = (omrpc_mxio_port_t *)hp->port;

        if(mport->handles[0] != hp) break;

        if(is_client){
            omrpc_read_nbytes(mport->fd,(char *)&s,sizeof(s));
            if(s != 1) mport->swap_flag = 1;

            s = 1;
            omrpc_write_nbytes(mport->fd,(char *)&s,sizeof(s));
        } else {
            s = 1;
            omrpc_write_nbytes(mport->fd,(char *)&s,sizeof(s));

            omrpc_read_nbytes(mport->fd,(char *)&s,sizeof(s));
            if(s != 1) mport->swap_flag = 1;
        }
        if(omrpc_debug_flag)
            omrpc_prf("byte order mxio %d...\n",mport->swap_flag);
        break;
    }
#ifdef USE_GLOBUS
    case PORT_GLOBUS_MXIO:
        omrpc_globus_io_handle_byte_order(hp,is_client);
        break;
#endif
    }

    if(is_client){
        omrpc_recv_short(hp,&s,1);
        omrpc_recv_done0(hp);
        if(s != 1) hp->swap_flag = 1;

        s = 1;
        omrpc_send_short(hp,&s,1);
        omrpc_send_done(hp);
    } else {
        s = 1;
        omrpc_send_short(hp,&s,1);
        omrpc_send_done(hp);
        omrpc_recv_short(hp,&s,1);
        omrpc_recv_done0(hp);
        if(s != 1) hp->swap_flag = 1;
    }
    if(omrpc_debug_flag)
        omrpc_prf("check byte order  done = %d...\n",hp->swap_flag);
}

/*
 * create socket to listen with port, return fd for accept
 */
int omrpc_io_socket(unsigned short *port)
{
    int sinlen;
    int r = -INT_MAX;
    struct sockaddr_in sin;
    struct hostent *hp;
    int fd;
    int one = 1;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        perror("socket failed");
        exit(1);
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(*port);

    hp = gethostbyname(omrpc_my_hostname);
    if(hp == NULL){
        herror("gethostbyname");
        omrpc_fatal("gethostbyname failure");
    }

    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sinlen = sizeof(sin);

     r = -INT_MAX;
     /*
     setsockopt(r, SOL_SOCKET, SO_REUSEADDR,
                (void *)&one, sizeof(int));
     */
     setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                (void *)&one, sizeof(int));
     setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,
                (char *)&one,sizeof(one));

    r = bind(fd, (struct sockaddr *)&sin, sizeof(sin));
    if (r < 0){
        perror("bind");
        omrpc_fatal("bind");
    }

    r = getsockname(fd,(struct sockaddr *)&sin,(socklen_t*)&sinlen);
    if(r < 0){
        perror("getsockname");
        omrpc_fatal("getsockname");
    }
    *port = ntohs(sin.sin_port);

    r = listen(fd,1);  /* set backlog */
    if (r < 0){
        perror("listen");
        omrpc_fatal("listen");
    }
    return fd;
}

int omrpc_io_accept(int fd)
{
    struct sockaddr_in sin;
    int sinlen,s;
    fd_set rfds;
    int max_nfd,nfd;
    struct timeval tm;

    FD_ZERO(&rfds);
    max_nfd = fd;
    FD_SET(fd,&rfds);
    tm.tv_sec = omrpc_timeout_sec; /* OMRPC_TIMEOUT_SEC_DEFAULT */
    tm.tv_usec = 0;
    nfd = select(max_nfd+1,&rfds,NULL,NULL,&tm);

    if(nfd != 1) omrpc_fatal("omrpc_io_accept: time out");
    if(!FD_ISSET(fd,&rfds))
        omrpc_fatal("omrpc_io_accept: select for acccept is failed, nfd=%d",
                    nfd);

    sinlen = sizeof(struct sockaddr_in);
    s = accept(fd,(struct sockaddr *)&sin,(socklen_t *)&sinlen);
    if(s < 0){
        if(omrpc_debug_flag) perror("accept failed");
        return s;
    }
    close(fd);
    return s;
}

int omrpc_io_connect(char *host, unsigned short port)
{
    int r;
    struct sockaddr_in sin;
    struct hostent *hp;
    int fd;
    int one = 1;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);

    if(host == NULL){
#if 0
        r = gethostname(hostname,MAXHOSTNAMELEN);
        if(r < 0){
            perror("hostname");
            omrpc_fatal("hostname");
        }
        host = hostname;
#endif
        host = "localhost";
    }

    if(omrpc_debug_flag) omrpc_prf("connect host=%s:%d\n",host,port);

    hp = gethostbyname(host);
    if(hp == NULL){
        herror("gethostbyname");
        omrpc_fatal("gethostbyname failure, gethostname host='%s'",host);
    }

    bcopy(hp->h_addr,&sin.sin_addr.s_addr,hp->h_length);

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        perror("socket failed");
        omrpc_fatal("socket");
    }
    setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char *)&one,sizeof(one));

    r = connect(fd,(struct sockaddr *)&sin,sizeof(sin));
    if(r < 0){
        perror("connect failed");
        omrpc_fatal("connect");
    }
    return fd;
}


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
