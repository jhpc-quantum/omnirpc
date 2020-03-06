static char rcsid[] = "$Id: omrpc_io_globus.c,v 1.1.1.1 2004-11-03 21:01:17 yoshihiro Exp $";
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
/* interfaces to GLOBUS */
#include "omrpc_defs.h"
#include "omrpc_io.h"
#include "omrpc_globus.h"
#include "omrpc_rpc.h"

globus_io_attr_t omrpc_globus_io_attr;
static globus_io_secure_authorization_data_t auth_data;
static globus_io_secure_authorization_mode_t auth_mode;

/* for GRAM */
static int no_gram_debug;
static globus_mutex_t gram_mutex;
static globus_cond_t gram_cond;
static globus_bool_t gram_done;
static char *gram_callback_contact;

static void gram_callback_func(void * user_callback_arg,
                               char * job_contact,
                               int state,
                               int errorcode);

void omrpc_init_globus_io()
{
    int r;

    if(omrpc_debug_flag) omrpc_prf("init_io_globus...\n");

    if(getenv("NO_GRAM_DEBUG") != NULL) no_gram_debug = TRUE;

    /* initialize globus IO */
    r = globus_module_activate(GLOBUS_COMMON_MODULE);
    if(r != GLOBUS_SUCCESS)
        omrpc_fatal("globus common module activation failed");
    r = globus_module_activate(GLOBUS_IO_MODULE);
    if(r != GLOBUS_SUCCESS)
        omrpc_fatal("globus io module activation failed");

    globus_io_tcpattr_init(&omrpc_globus_io_attr);
    globus_io_attr_set_tcp_nodelay(&omrpc_globus_io_attr,GLOBUS_TRUE);
    globus_io_secure_authorization_data_initialize(&auth_data);

    if(!no_gram_debug && omrpc_is_client){
        /* initialize gram */
        r = globus_module_activate(GLOBUS_GRAM_CLIENT_MODULE);
        if(r != GLOBUS_SUCCESS)
            omrpc_fatal("globus gram module activation failed");

        globus_mutex_init(&gram_mutex, (globus_mutexattr_t *) NULL);
        globus_cond_init(&gram_cond, (globus_condattr_t *) NULL);
        gram_done = GLOBUS_FALSE;

        r = globus_gram_client_callback_allow(gram_callback_func,
                                              (void *) NULL,
                                              &gram_callback_contact);
        if(r != GLOBUS_SUCCESS)
            omrpc_fatal("globus_gram_client_callback_allow failed");
    }

    if(omrpc_debug_flag) omrpc_prf("omrpc_globus_io end ...\n");
}

void omrpc_finalize_globus_io()
{
    /* finalize globus IO */
    globus_io_tcpattr_destroy(&omrpc_globus_io_attr);
    globus_module_deactivate_all();

    globus_module_deactivate(GLOBUS_IO_MODULE);

    if(!no_gram_debug){
        /* finalize gram */
        globus_mutex_destroy(&gram_mutex);
        globus_cond_destroy(&gram_cond);

        /* Deactivate GRAM */
        globus_module_deactivate(GLOBUS_GRAM_CLIENT_MODULE);
    }
}

int omrpc_exec_by_gram(char *exec_host,
                       char *path,
                       char *host,
                       unsigned short port_num,
                       int mxio_flag,
                       char *job_sched_type,
                       char *reg_path,
                       void **jobs_info)
{
    int pid;

    if(omrpc_debug_flag) 
        omrpc_prf("exec_by_gram ... %s/%s:%s:%d from %s\n",
                  exec_host, OMRPC_GLOBUS_JOB_MANAGER, path,port_num,host);

    if(no_gram_debug){  /* for debug in local globus */
        int ac;
        char *arg[30];
        char port_arg[10];
        char *prog;

        snprintf(port_arg, sizeof(port_arg), "%d",port_num);
        ac = 0;
        prog = "rsh";
        arg[ac++] = "rsh";
        arg[ac++] = exec_host;
        arg[ac++] = path;
        if(omrpc_debug_flag) arg[ac++] = "-debug";
        arg[ac++] = "-globus";
        arg[ac++] = "-host";
        arg[ac++] = host;
        if(mxio_flag) arg[ac++] = "-mxio";
        if(job_sched_type != NULL){
            arg[ac++] = "-sched";
            arg[ac++] = job_sched_type;
        }
        if(reg_path != NULL){
            arg[ac++] = "-reg";
            arg[ac++] = reg_path;
        }
        arg[ac++] = "-port";
        arg[ac++] = port_arg;
        arg[ac++] = NULL;

        if((pid = fork()) == 0){
            execvp(prog,arg);
            if(omrpc_debug_flag) perror("execvp failed:");
            exit(1);
        }
        return pid;
    } else {
        int rc;
        char *job_contact;
        char specification[512];
        char job_sched_arg[20];
        int job_state_mask = GLOBUS_GRAM_PROTOCOL_JOB_STATE_ALL;
        int exec_host_contact_len;
        char *exec_host_contact;
        char reg_path_arg[512];

        exec_host_contact_len = strlen(exec_host)+
            strlen(OMRPC_GLOBUS_JOB_MANAGER) + 1; /* add "/" before jobmanager type */
        exec_host_contact = (char*)omrpc_malloc(exec_host_contact_len+1);
        strncpy(exec_host_contact, exec_host,exec_host_contact_len);
        strncat(exec_host_contact, "/", exec_host_contact_len);
        strncat(exec_host_contact, OMRPC_GLOBUS_JOB_MANAGER, exec_host_contact_len);

        if(job_sched_type != NULL){
            snprintf(job_sched_arg, sizeof(job_sched_arg), 
                     "-sched %s",job_sched_type);
        } else job_sched_arg[0] = '\0';

        if(reg_path != NULL){
            snprintf(reg_path_arg, sizeof(reg_path_arg), 
                     "-reg %s", reg_path);
        } else reg_path_arg[0] = '\0';

        if(omrpc_debug_flag){
            snprintf(specification, sizeof(specification),
                     "&(executable=%s)"
                     "(arguments= -debug -globus -host %s %s %s %s -port %d)"
                     "(stdout=omrpc.stdout)(stderr=omrpc.stderr)",
                     path,host,mxio_flag? "-mxio":"",
                     job_sched_arg, reg_path_arg,port_num);
        } else {
            snprintf(specification, sizeof(specification),
                    "&(executable=%s)(arguments= -globus -host %s %s %s %s -port %d)",
                    path,host,mxio_flag? "-mxio":"",
                    job_sched_arg, reg_path_arg, port_num);
        }

        rc = globus_gram_client_job_request(exec_host_contact,/* rm_contact */
                                            specification,
                                            job_state_mask,
                                            gram_callback_contact,
                                            &job_contact);
        if (rc != 0){ /* if there is an error */
            omrpc_prf("gram subumit error: %d - %s\n",
                      rc, globus_gram_client_error_string(rc));
            omrpc_fatal("gram submit failed");
        }
        *jobs_info = (void *)job_contact;
        globus_mutex_lock(&gram_mutex);
        while (!gram_done){
            globus_cond_wait(&gram_cond, &gram_mutex);
        }
        globus_mutex_unlock(&gram_mutex);
        omrpc_free(exec_host_contact);
        gram_done = GLOBUS_FALSE;
        return 0;       /* pid */
    }
}

static void gram_callback_func(void * user_callback_arg,
                               char * job_contact,
                               int state,
                               int errorcode)
{
    switch(state){
    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_PENDING:
        if(omrpc_debug_flag) omrpc_prf("gram pending\n");
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_ACTIVE:
        if(omrpc_debug_flag) omrpc_prf("gram active\n");
        globus_mutex_lock(&gram_mutex);
        gram_done = GLOBUS_TRUE;
        globus_cond_signal(&gram_cond);
        globus_mutex_unlock(&gram_mutex);
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_FAILED:
        omrpc_fatal("gram_job failed");
        break;

    case GLOBUS_GRAM_PROTOCOL_JOB_STATE_DONE:
        if(omrpc_debug_flag) omrpc_prf("gram done\n");
        break;
    }
}


omrpc_io_handle_t *omrpc_globus_io_handle(int mxio_flag)
{
    omrpc_io_handle_t *hp;

    hp = (omrpc_io_handle_t *)omrpc_malloc(sizeof(omrpc_io_handle_t));

    if(!mxio_flag){
        omrpc_globus_io_port_t *port;
        port = (omrpc_globus_io_port_t *)
            omrpc_malloc(sizeof(omrpc_globus_io_port_t));
        port->port_type = PORT_GLOBUS;
        hp->port = (omrpc_io_port_t *)port;
        port->hp = hp;
    } else {
	omrpc_globus_mxio_port_t *mport;
	mport = (omrpc_globus_mxio_port_t *)
	    omrpc_malloc(sizeof(omrpc_globus_mxio_port_t));
	mport->port_type = PORT_GLOBUS_MXIO;
	hp->port = (omrpc_io_port_t *)mport;
	hp->port_n = 0;
	mport->handles[0] = hp;

	globus_mutex_init(&mport->lock,NULL);
	globus_cond_init(&mport->cond,NULL);

    }
    return hp;
}

void omrpc_globus_listener(globus_io_handle_t *handle,
			   unsigned short *port)
{
    globus_result_t result;

    *port = 0;
    result = globus_io_tcp_create_listener(port, 1, &omrpc_globus_io_attr, 
					   handle);
    if (result != GLOBUS_SUCCESS){
	omrpc_fatal("omrpc_globus_listener error");
    }
}

void omrpc_globus_io_accept(globus_io_handle_t *listener_handle,
			    omrpc_io_handle_t *hp)
{
    globus_result_t result;
    omrpc_io_port_t *port;

    result = globus_io_tcp_listen(listener_handle);
    if (result != GLOBUS_SUCCESS) {
	omrpc_fatal("omrpc_globus_io_accept: tcp_listen error");
     }

    port = hp->port;
    switch(port->port_type){
    case PORT_GLOBUS:
	result = 
	    globus_io_tcp_accept(listener_handle, &omrpc_globus_io_attr,
				 &((omrpc_globus_io_port_t *)port)->g_handle);
	break;
    case PORT_GLOBUS_MXIO:
	result = 
	    globus_io_tcp_accept(listener_handle, &omrpc_globus_io_attr,
				 &((omrpc_globus_mxio_port_t *)port)->g_handle);
	break;
    default:
	omrpc_fatal("omrpc_globus_io_accept: bad port type");
    }

    if (result != GLOBUS_SUCCESS){
	omrpc_fatal("omrpc_globus_io_accept: tcp_accept error");
    }
    globus_io_close(listener_handle);
}

void omrpc_globus_read_mxio_header(omrpc_globus_mxio_port_t *port,
				   int *port_n, int *nbytes)
{
    globus_result_t result;
    short b[2];
    int r;
    result = globus_io_read(&port->g_handle,(char *)b,sizeof(b),sizeof(b),&r);
    if (result != GLOBUS_SUCCESS || r != sizeof(b)){
	globus_object_t *  err = globus_error_get(result);
        if(globus_io_eof(err)){
	    *port_n = 0;
	    *nbytes = 0;
	    return;
	} else 
	    omrpc_fatal("error in globus_io_read mxio header");
    }
    *port_n = b[0];
    *nbytes = b[1];
}

void omrpc_globus_write_mxio_header(omrpc_globus_mxio_port_t *port,
				    int port_n, int nbytes)
{
    globus_result_t result;
    short b[2];
    int r;

    if(nbytes == 0) return;

    b[0] = port_n;
    b[1] = nbytes;
    result = globus_io_write(&port->g_handle,(char *)b,sizeof(b),&r);
    if (result != GLOBUS_SUCCESS || r != sizeof(b)){
	omrpc_fatal("error in globus_io_write mxio header");
    }
}

void omrpc_globus_send_mxio_ack(omrpc_globus_mxio_port_t *port,char cmd)
{
    globus_result_t result;
    int r;

    omrpc_globus_write_mxio_header(port,0,1);
    result = globus_io_write(&port->g_handle,&cmd,1,&r);
    if (result != GLOBUS_SUCCESS || r != 1){
	omrpc_fatal("error in globus_io_write mxio ack");
    }
}

void omrpc_globus_io_handle_fill(omrpc_io_handle_t *hp)
{
    globus_result_t result;
    omrpc_globus_io_port_t *port = (omrpc_globus_io_port_t *)hp->port;
    int r;

    switch(port->port_type){
    case PORT_GLOBUS:
	result = globus_io_read(&port->g_handle,hp->buf,
				OMRPC_IO_BUF_SIZE,1,&r);
	if (result != GLOBUS_SUCCESS){
	    globus_object_t *  err = globus_error_get(result);
	    if(globus_io_eof(err)){
		hp->ncount = 0;
	    } else {
		omrpc_prf("async_flag=%d\n",port->async_flag);
		omrpc_prf("error string='%s'\n",
			  globus_error_generic_string_func(err));
		omrpc_fatal("error in globus_io_read\n");
	    }
	} else 
	    hp->ncount = r;
	hp->pos = 0;
	return;
	
    case PORT_GLOBUS_MXIO:
    {
	int port_n,nbytes;
	omrpc_globus_mxio_port_t *mport = (omrpc_globus_mxio_port_t *)port;
	globus_result_t result;

	pthread_mutex_lock(&mport->lock);
	if(mport->active_flag){
	    if(hp->port_n == 0){
		/* if port is active and request for port_n 0, 
		   and not peeked yet,
		   then wait for input */
		while(!hp->peeked) 
		  globus_cond_wait(&mport->cond,&mport->lock);
		hp->peeked = FALSE;
		hp->pos = 0;
		hp->ncount = hp->peeked_ncount;
		pthread_mutex_unlock(&mport->lock);
		return;
	    } else
		omrpc_fatal("fill for active globus mx port");
	}

	/* read header */
	omrpc_globus_read_mxio_header(mport,&port_n,&nbytes);
	if(port_n != hp->port_n)
	    omrpc_fatal("invalid message in globus MXIO, %d != %d",
			port_n, hp->port_n);
	hp->ncount = nbytes;
	result = globus_io_read(&port->g_handle,hp->buf,nbytes,nbytes,&r);
	if (result != GLOBUS_SUCCESS || r != nbytes)
	    omrpc_fatal("error in globus_io_read mxio");
	hp->pos = 0;
	pthread_mutex_unlock(&mport->lock);
	return;
    }
    default:
	omrpc_fatal("omrpc_globus_io_fill: bad port type %d",port->port_type);
    }
}

void omrpc_globus_io_handle_flush(omrpc_io_handle_t *hp)
{
    globus_result_t result;
    omrpc_globus_io_port_t *port = (omrpc_globus_io_port_t *)hp->port;
    int r;

    switch(port->port_type){
    case PORT_GLOBUS:
	result = globus_io_write(&port->g_handle,hp->buf,hp->pos,&r);
	if (result != GLOBUS_SUCCESS)
	    omrpc_fatal("error in globus_io_write");
	if(hp->pos != r) 
	    omrpc_fatal("bad nbyte count in globus_io_write");
	hp->pos = 0;
	return;

    case PORT_GLOBUS_MXIO:
    {
	omrpc_globus_mxio_port_t *mport = (omrpc_globus_mxio_port_t *)port;

	if(omrpc_debug_flag) 
	    omrpc_prf("flush globus mxio port=%d,nbyte=%d\n", 
		      hp->port_n,hp->pos);

	if(hp->pos == 0) return;
	omrpc_io_lock();
	omrpc_globus_write_mxio_header(mport,hp->port_n,hp->pos);
	result = globus_io_write(&port->g_handle,hp->buf,hp->pos,&r);
	if (result != GLOBUS_SUCCESS || r != hp->pos){
	    omrpc_fatal("error in globus_io_write mxio");
	}
	hp->pos = 0;
	omrpc_io_unlock();
	return;
    }
    default:
	omrpc_fatal("omrpc_globus_io_flush: bad port type %d",port->port_type);
    }
}

/* close */
void  omrpc_globus_io_handle_close(omrpc_io_handle_t *hp)
{
    omrpc_globus_io_port_t *port = (omrpc_globus_io_port_t *)hp->port;

    switch(port->port_type){
    case PORT_GLOBUS:
	globus_io_cancel(&port->g_handle,GLOBUS_FALSE);
	globus_io_close(&port->g_handle);
	omrpc_free(port);
	omrpc_free(hp);
	return;
	
    case PORT_GLOBUS_MXIO:
    {
	omrpc_globus_mxio_port_t *mport = (omrpc_globus_mxio_port_t *)port;
	mport->handles[hp->port_n] = NULL;
	omrpc_free(hp);
	return;
    }
    default:
	omrpc_fatal("omrpc_globus_io_handle_close: bad port type %d",
		    port->port_type);
    }
}

void omrpc_globus_io_connect(char *hostname, unsigned short port_n, 
			     omrpc_io_port_t *port)
{
    globus_result_t result;
    result = 
	globus_io_tcp_connect(hostname,port_n,&omrpc_globus_io_attr,
			      &((omrpc_globus_io_port_t *)port)->g_handle);
    if (result != GLOBUS_SUCCESS){
        omrpc_fatal("globus_io_tcp_connect failed");
    }
}

void omrpc_globus_io_handle_byte_order(omrpc_io_handle_t *hp,int is_client)
{
    short s;
    int r;
    globus_result_t result;
    omrpc_globus_mxio_port_t *mport = (omrpc_globus_mxio_port_t *)hp->port;

    if(omrpc_debug_flag) 
	omrpc_prf("globus_io_byte_order... is_client=%d\n",is_client);
    
    if(mport->handles[0] != hp) return;

    if(is_client){
	result = globus_io_read(&mport->g_handle,(char *)&s,
				sizeof(s),sizeof(s),&r);
	if(result != GLOBUS_SUCCESS){
	    omrpc_prf("error in read\n");
	    goto err;
	}
	if(s != 1) mport->swap_flag = 1;
	
	s = 1;
	result = globus_io_write(&mport->g_handle,(char *)&s,sizeof(s),&r);
	if(result != GLOBUS_SUCCESS){
	    omrpc_prf("error in write\n");
	    goto err;
	}
    } else {
	s = 1;
	result = globus_io_write(&mport->g_handle,(char *)&s,sizeof(s),&r);
	if(result != GLOBUS_SUCCESS) goto err;
	
	result = globus_io_read(&mport->g_handle,(char *)&s,
				sizeof(s),sizeof(s),&r);
	if(result != GLOBUS_SUCCESS) goto err;
	if(s != 1) mport->swap_flag = 1;
    }
    if(omrpc_debug_flag) 
	omrpc_prf("globus_io_byte_order... done swap_flag=%d\n",
		  mport->swap_flag);
    return;
err:
    omrpc_fatal("globus io error in byte_order check");
}

#endif
