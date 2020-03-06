static char rcsid[] = "$Id: omrpc_globus.c,v 1.1.1.1 2004-11-03 21:01:15 yoshihiro Exp $";
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

#ifdef USE_GLOBUS

/* 
 * interfaces to GLOBUS : job submit, io for rpc
 */
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_globus.h"
#include "omrpc_rpc.h"

extern globus_io_attr_t omrpc_globus_io_attr;

/*  globus IO callback func prototpye */
static void listen_callback_func(void *arg,globus_io_handle_t *listener_handle,
				 globus_result_t result);
static void accept_callback_func(void *arg,globus_io_handle_t *handle,
				 globus_result_t result);
static void read_callback_func(void *arg,globus_io_handle_t *handle,
			       globus_result_t result,
			       globus_byte_t *buf, globus_size_t nbytes);

/* omrpc handler for globus port */
static void *omrpc_globus_rpc_handler_main(void *dummy);
static int handler_running = FALSE;

static globus_mutex_t handler_mutex;
static globus_cond_t waiting_cond,hold_cond;
static omrpc_io_handle_t *input_hp;

void omrpc_init_globus(int rpc_flag)
{
    int r;
    static pthread_attr_t pth_attr;
    static pthread_t tid;

    if(omrpc_debug_flag) omrpc_prf("init globus... rpc_flag=%d\n",rpc_flag);

    if(rpc_flag){
	/* create thread for globus rpc handler */
	globus_mutex_init(&handler_mutex, (globus_mutexattr_t *) NULL);
	globus_cond_init(&waiting_cond, (globus_condattr_t *) NULL);
	globus_cond_init(&hold_cond, (globus_condattr_t *) NULL);
	input_hp = NULL;

	pthread_attr_init(&pth_attr);
	r = pthread_create(&tid, &pth_attr, omrpc_globus_rpc_handler_main, NULL);
	if(r != 0){
	    omrpc_fatal("cannot create globus handler thread");
	}
	handler_running = TRUE;
    }

    if(omrpc_debug_flag) omrpc_prf("omrpc_globus_init end ...\n");

}

void omrpc_finalize_globus()
{
}

omrpc_io_handle_t *omrpc_globus_io_handle_create(omrpc_io_handle_t *hp)
{
    globus_result_t result;
    omrpc_globus_io_port_t *port = (omrpc_globus_io_port_t *)hp->port;
    omrpc_io_handle_t *new_hp;

    switch(port->port_type){
    case PORT_GLOBUS:
    {
	unsigned short port_n = 0;

	omrpc_io_lock();
	new_hp = omrpc_globus_io_handle(FALSE);
	omrpc_io_unlock();

	port = (omrpc_globus_io_port_t *)new_hp->port;
	omrpc_globus_listener(&port->listener_handle,&port_n);
	new_hp->port_n = port_n;

	/* initiate call back */
	result = globus_io_tcp_register_listen(&port->listener_handle,
					       listen_callback_func,
					       port);
	if (result != GLOBUS_SUCCESS) {
	    omrpc_fatal("omrpc_globus_io_accept: tcp_register_listen error");
	}

	if(omrpc_debug_flag) omrpc_prf("register listen...\n");
	return new_hp;
    }
    case PORT_GLOBUS_MXIO:
    {
	int i;
	omrpc_globus_mxio_port_t *mport = (omrpc_globus_mxio_port_t *)port;

	omrpc_io_lock();
	for(i = 1; i < MAX_HANDLE_PER_PORT; i++){
	    if(mport->handles[i] == NULL) break;
	}
	if(i == MAX_HANDLE_PER_PORT)
	    omrpc_fatal("too many mxio port");
	hp = omrpc_malloc(sizeof(omrpc_io_handle_t));
	hp->port = (omrpc_io_port_t *)port;
	hp->port_n = i;
	mport->handles[i] = hp;
	omrpc_io_unlock();

	return hp;
    }
    default:
	omrpc_fatal("omrpc_globus_io_handle_create: bad port type %d",
		    port->port_type);
    }
}

/*
 * call back functions for globus API
 */
static void listen_callback_func(void *arg,globus_io_handle_t *listener_handle,
				 globus_result_t result)
{
    omrpc_globus_io_port_t *port = (omrpc_globus_io_port_t *)arg;

    if(omrpc_debug_flag) omrpc_prf("listener callback...\n");

    if (result != GLOBUS_SUCCESS)
	omrpc_fatal("listen_callback_func returns error");
    
    result = globus_io_tcp_register_accept(listener_handle, 
					   &omrpc_globus_io_attr,
					   &port->g_handle,
					   accept_callback_func,
					   (void *)port);
    if (result != GLOBUS_SUCCESS)
	omrpc_fatal("error in setting accept_callback_func");
}

static void accept_callback_func(void *arg,globus_io_handle_t *handle,
				 globus_result_t result)
{
    omrpc_globus_io_port_t *port = (omrpc_globus_io_port_t *)arg;

    if(omrpc_debug_flag) omrpc_prf("accept callback...\n");

    if (result != GLOBUS_SUCCESS)
	omrpc_fatal("accept_callback_func return error");
    
    globus_io_close(&port->listener_handle);	/* close listener */

    port->async_flag = TRUE;

    /* set read_callback */
    result = globus_io_register_read(handle,port->hp->buf,
				     OMRPC_IO_BUF_SIZE,1,
                                     read_callback_func,arg);
    if (result != GLOBUS_SUCCESS)
	omrpc_fatal("error in setting read_callback_func");
}

static void read_callback_func(void *arg,globus_io_handle_t *handle,
			       globus_result_t result,
			       globus_byte_t *buf, globus_size_t nbytes)
{
    omrpc_globus_io_port_t *port = (omrpc_globus_io_port_t *)arg;
    omrpc_io_handle_t *hp;
    
    if(omrpc_debug_flag) omrpc_prf("read callback...\n");

    hp = port->hp;
    if (result != GLOBUS_SUCCESS){
        globus_object_t *  err = globus_error_get(result);
        if(!globus_io_eof(err)){
	    omrpc_fatal("error in read_callback");
	}
	port->async_flag = FALSE;
	nbytes = 0;
    } 
    hp->peeked = TRUE;
    hp->peeked_ncount = nbytes;

    globus_mutex_lock(&handler_mutex);
    while(input_hp != NULL) globus_cond_wait(&hold_cond,&handler_mutex);
    input_hp = hp;
    globus_cond_signal(&waiting_cond);
    globus_mutex_unlock(&handler_mutex);
}


static void read_mx_callback_func(void *arg,globus_io_handle_t *handle,
				  globus_result_t result,
				  globus_byte_t *buf, globus_size_t nbytes)
{
    omrpc_globus_mxio_port_t *port = (omrpc_globus_mxio_port_t *)arg;
    omrpc_io_handle_t *hp;
    short *hbuf = (short *)buf;
    int port_n,r;

    if(omrpc_debug_flag) omrpc_prf("read mx callback...\n");

    if (result != GLOBUS_SUCCESS && nbytes != sizeof(short)*2){
        globus_object_t *  err = globus_error_get(result);
        if(!globus_io_eof(err)){
	    omrpc_fatal("error in read_callback");
	}
	nbytes = 0;
    } 

    port_n = hbuf[0];
    nbytes = hbuf[1];
    hp = port->handles[port_n];

    pthread_mutex_lock(&port->lock);

    /* read body */
    if(hp->peeked) omrpc_fatal("error in read_mx_callback, ready peeked");
    if(nbytes != 0){
	result = globus_io_read(&port->g_handle,hp->buf, nbytes,nbytes,&r);
	if (result != GLOBUS_SUCCESS || r != nbytes)
	    omrpc_fatal("error in read_mx_callback, read body");
    }

    if(port_n == 0){	/* input for port_n == 0 */
	globus_cond_signal(&port->cond);
    }
    hp->peeked_ncount = nbytes;
    hp->peeked = TRUE;		/* at last, set flag */
    port->active_flag = FALSE;
    pthread_mutex_unlock(&port->lock);

    if(port_n == 0) return;

    globus_mutex_lock(&handler_mutex);
    while(input_hp != NULL) globus_cond_wait(&hold_cond,&handler_mutex);
    input_hp = hp;
    globus_cond_signal(&waiting_cond);
    globus_mutex_unlock(&handler_mutex);
}

static void *omrpc_globus_rpc_handler_main(void *dummy)
{
    omrpc_io_handle_t *hp;
    omrpc_rpc_t *rp;

again:
    globus_mutex_lock(&handler_mutex);
    while(input_hp == NULL) 
	globus_cond_wait(&waiting_cond,&handler_mutex);
    hp = input_hp;
    input_hp = NULL;
    globus_cond_signal(&hold_cond);
    globus_mutex_unlock(&handler_mutex);

    if(omrpc_debug_flag) 
	omrpc_prf("globus handler input_hp = 0x%x rp=0x%x\n",hp,hp->hint);

    if((rp = (omrpc_rpc_t *)hp->hint) != NULL && rp->req != NULL)
	omrpc_rpc_handler(rp->req);
    
    goto again;	/* loop forever */
}

/* set handler again */
void omrpc_globus_recv_done(omrpc_io_port_t *p)
{
    if(!handler_running) return;

    if(omrpc_debug_flag) omrpc_prf("omrpc_globus_recv_done ...\n");

    switch(p->port_type){
    case PORT_GLOBUS:
    {
	omrpc_globus_io_port_t *port = (omrpc_globus_io_port_t *)p;
	globus_result_t result;
    
	if(omrpc_debug_flag) 
	    omrpc_prf("globus_recv_done async_flag=%d\n",port->async_flag);

	if(!port->async_flag) return;
	
	/* set read_callback */
	result = globus_io_register_read(&port->g_handle,port->hp->buf,
					 OMRPC_IO_BUF_SIZE,1,
					 read_callback_func,(void *)port);
	if (result != GLOBUS_SUCCESS)
	    omrpc_fatal("error in setting read_callback_func");
	return;
    }
    case PORT_GLOBUS_MXIO:
    {
	omrpc_globus_mxio_port_t *port = (omrpc_globus_mxio_port_t *)p;
	globus_result_t result;

	/* set read_callback */
	result = globus_io_register_read(&port->g_handle,(char *)port->hbuf,
					 sizeof(short)*2,sizeof(short)*2,
					 read_mx_callback_func,(void *)port);
	if (result != GLOBUS_SUCCESS)
	    omrpc_fatal("error in setting read_callback_func");
	port->active_flag = TRUE;
	if(omrpc_debug_flag) omrpc_prf("set callback MXIO ...\n");
	return;
    }
    default:
	omrpc_fatal("omrpc_globus_recv_done: bad port type %d",
		    p->port_type);
    }
}

#endif


