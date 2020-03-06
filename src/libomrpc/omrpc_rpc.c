/*
 * $Id: omrpc_rpc.c,v 1.2 2006-01-25 16:06:18 ynaka Exp $
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
#include "omrpc_defs.h"
#include "omrpc_rpc.h"
#include "omrpc_agent.h"

int omrpc_rpc_counter = 0;

static rpc_queue_t active_queue, idle_queue;   /* active queue is not used */

static omrpc_rpc_t *free_rpc_obj = NULL;       /* free list */
static omrpc_request_t *free_req = NULL;

static pthread_mutex_t rpc_lock;       /* general lock for omrpc_rpc */
#define RPC_LOCK        pthread_mutex_lock(&rpc_lock)
#define RPC_UNLOCK      pthread_mutex_unlock(&rpc_lock)

static pthread_mutex_t agent_lock;      /* general lock for manger task */
#define AGENT_LOCK      pthread_mutex_lock(&agent_lock)
#define AGENT_UNLOCK    pthread_mutex_unlock(&agent_lock)

static void omrpc_queue_put(rpc_queue_t *queue,omrpc_rpc_t *rp);
static void omrpc_queue_remove(rpc_queue_t *queue,omrpc_rpc_t *rp);
static char *omrpc_req_kind_name(int kind);

#ifdef USE_FT
extern void omrpcm_io_handle_close(omrpc_mpi_handle_t **mptr);
extern void omrpc_io_handle_close(omrpc_io_handle_t *hp); 
extern pthread_cond_t  omrpc_mpi_cond;
extern pthread_mutex_t omrpc_mpi_mutex;
#endif

/*
 * initialize staff in this module
 */
void omrpc_rpc_init()
{
    /* initialize mutex */
    pthread_mutex_init(&rpc_lock,NULL);
    pthread_mutex_init(&agent_lock,NULL);
    idle_queue.head = active_queue.head = NULL;

    if(omrpc_debug_flag) omrpc_prf("omrpc_rpc_init end ...\n");
}

/* init idle_queue & active_queue (used only by MPI-ver) */
void omrpcm_init_queue()
{
  idle_queue.head = active_queue.head = NULL;
} /* omrpcm_init_queue */

/* allocate rpc object. DO NOT use in MT */
omrpc_rpc_t *omrpc_allocate_rpc_obj()
{
    omrpc_rpc_t *rp;
    rp = (omrpc_rpc_t *)omrpc_malloc(sizeof(omrpc_rpc_t));
    memset(rp, 0, sizeof(omrpc_rpc_t));
    return rp;
}

omrpc_rpc_t *omrpc_new_rpc()
{
    omrpc_rpc_t *rp;
    RPC_LOCK;
    if((rp = free_rpc_obj) == NULL){
        rp = omrpc_allocate_rpc_obj();
    }else{
        free_rpc_obj = rp->next;
        bzero(rp,sizeof(omrpc_rpc_t));
    }
    rp->id = ++omrpc_rpc_counter;
    if(omrpc_tlog_flag) tlog_RPC_IN(rp->id);

    RPC_UNLOCK;
    return rp;
}

void omrpc_delete_rpc(omrpc_rpc_t *rp)
{
    RPC_LOCK;
    if(omrpc_tlog_flag) tlog_RPC_OUT(rp->id);
    rp->next = free_rpc_obj;
    free_rpc_obj = rp;
    RPC_UNLOCK;
}

/*
 * queue management
 */
static void omrpc_queue_remove(rpc_queue_t *queue,omrpc_rpc_t *rp)
{
    if(rp == queue->head){      /* head */
        queue->head = rp->next;
        if(rp->next != NULL) rp->next->prev = NULL;
    } else if(rp == queue->tail){       /* tail */
        queue->tail = rp->prev;
        if(rp->prev != NULL) rp->prev->next = NULL;
    } else {
        rp->prev->next = rp->next;
        rp->next->prev = rp->prev;
    }
}

/* put at tail */
static void omrpc_queue_put(rpc_queue_t *queue,omrpc_rpc_t *rp)
{
    if(queue->head == NULL){    /* empty */
        queue->head = queue->tail = rp;
        rp->next = rp->prev = NULL;
    } else {
        queue->tail->next = rp;
        rp->prev = queue->tail;
        rp->next = NULL;
        queue->tail = rp;
    }
}

/*
 * idle queue
 */
omrpc_rpc_t *omrpc_get_idle_rpc(omrpc_module_t *module)
{
    omrpc_rpc_t *rp;
    RPC_LOCK;

    for(rp = idle_queue.head; rp != NULL; rp = rp->next){
      if(rp->nprocs != module->nprocs) continue; // Skip if nprocs is not equall to the one requred
        if(rp->module == module){       /* remote from queue */
            omrpc_queue_remove(&idle_queue,rp);
            break;
        }
    }

    RPC_UNLOCK;
    return rp;
}

omrpc_rpc_t *
omrpc_get_victim_idle_rpc(omrpc_host_t *host, omrpc_module_t *module)
{
    omrpc_rpc_t *rp;
    RPC_LOCK;
    for(rp = idle_queue.head; rp != NULL; rp = rp->next){
        if(rp->host == host && rp->module != module){
            omrpc_queue_remove(&idle_queue,rp);
            break;
        }
    }
    RPC_UNLOCK;
    return rp;
}

omrpc_rpc_t *
omrpcm_get_victim_idle_rpc(omrpc_host_t *host, omrpc_module_t *module)
{
    omrpc_rpc_t *rp;

    //RPC_LOCK;

    for(rp = idle_queue.head; rp != NULL; rp = rp->next){
        if(rp->host == host && rp->module != module){
            omrpc_queue_remove(&idle_queue,rp);
            break;
        }
        if(rp->host == host && rp->module == module && rp->nprocs!=module->nprocs){
            omrpc_queue_remove(&idle_queue,rp);
            break;
        }
    }

    //RPC_UNLOCK;
    return rp;
}


void omrpc_put_idle_rpc(omrpc_rpc_t *rp)
{
    RPC_LOCK;
    omrpc_queue_put(&idle_queue,rp);    /* put in idle */
    omrpc_release_job(-1);
    RPC_UNLOCK;
}

omrpc_rpc_t *omrpc_exec_module(omrpc_host_t *host,omrpc_module_t *module)
{
    omrpc_rpc_t *rp;
    omrpc_io_handle_t *agent_hp;
    omrpc_request_t *req;

    rp = omrpc_new_rpc();
    rp->host = host;
    rp->module = module;
    agent_hp = host->agent_rp->hp;
  
    /* create request and activate */
    req = omrpc_new_req(rp,OMRPC_INVOKE);
    if(omrpc_tlog_flag) tlog_INVOKE_IN(rp->id);

    if(omrpc_debug_flag)
        omrpc_prf("exec_module...host=%s, module=%s\n",
                  host->name, module->name);

    RPC_LOCK;
    rp->hp = omrpc_io_handle_create(agent_hp);
    rp->hp->hint = (void *)rp;

    if(omrpc_debug_flag)
        omrpc_prf("invoke handler input_hp = %p rp=%p\n",
                  rp->hp,rp->hp->hint);

    RPC_UNLOCK;

    AGENT_LOCK;
    /* exec by manager */
    omrpc_agent_invoke(agent_hp,omrpc_rex_prog_path(host,module),
                       omrpc_io_handle_port_n(rp->hp),module->nprocs);
    AGENT_UNLOCK;

    if(omrpc_debug_flag) omrpc_prf("exec_module...invoke done\n");

    /* wait */
    omrpc_request_wait_done(req);
    /* if have initialize method */
    if(rp->module->have_init){
        /* create request and activate */
        req = omrpc_new_req(rp,OMRPC_CALL_INIT);
        if(omrpc_tlog_flag) tlog_CALL_INIT_IN(rp->id);

        if(omrpc_tlog_flag) tlog_SEND_ARG_IN(rp->id);
        req->stub = rp->module->init_stub;
        omrpc_call_send_args(rp->hp,rp->module->init_stub->stub_info,
                             rp->module->init_args);
        if(omrpc_tlog_flag) tlog_SEND_ARG_OUT(rp->id);

        /* wait */
        omrpc_request_wait_done(req);
    }

    return rp;
}

void omrpc_module_init_setup(omrpc_module_t *mp,va_list *app)
{
    omrpc_host_t *host;
    omrpc_rpc_t *rp;
    omrpc_io_handle_t *agent_hp;
    omrpc_request_t *req;
    char *rex_path;

    /* take first rex entry */
    host = mp->rex_list->host;
    rex_path = mp->rex_list->path;

    rp = omrpc_new_rpc();
    rp->host = host;
    rp->module = mp;
    agent_hp = host->agent_rp->hp;

    /* create request and activate */
    req = omrpc_new_req(rp,OMRPC_INVOKE);
    if(omrpc_tlog_flag) tlog_INVOKE_IN(rp->id);

    RPC_LOCK;
    rp->hp = omrpc_io_handle_create(agent_hp);
    rp->hp->hint = (void *)rp;
    RPC_UNLOCK;

    AGENT_LOCK;
    /* exec by manager */
    omrpc_agent_invoke(agent_hp,rex_path, omrpc_io_handle_port_n(rp->hp),mp->nprocs);
    AGENT_UNLOCK;

    /* wait for invoke */
    omrpc_request_wait_done(req);

    req = omrpc_new_req(rp,OMRPC_STUB_INFO);
    if(omrpc_tlog_flag) tlog_STUB_INFO_IN(rp->id);

    req->stub = mp->init_stub;
    omrpc_req_stub_info(rp->hp,"Initialize");
    omrpc_request_wait_done(req);

    if(mp->init_stub->stub_info == NULL)
        omrpc_fatal("omrpc_module_init: stub NULL");

    /* this module only get information about initialization, so kill rpc */
    omrpc_kill_rpc(rp);

    omrpc_setup_args(app,mp->init_args,mp->init_stub->stub_info);
    mp->have_init = TRUE;

    if(omrpc_debug_flag) omrpc_prf("module_init_setup end ...\n");

}

omrpc_request_t *omrpc_call_submit(omrpc_rpc_t *rp,
                                   omrpc_stub_entry_t *sp, va_list *app)
{
    omrpc_request_t *req;

    if(omrpc_debug_flag) omrpc_prf("omrpc_call_submit ... rp=%p\n",rp);

    if(sp->stub_info == NULL){
        if(omrpc_debug_flag) omrpc_prf("omrpc_call_submit ... req stub\n");
        req = omrpc_new_req(rp,OMRPC_STUB_INFO);
        if(omrpc_tlog_flag) tlog_STUB_INFO_IN(rp->id);

        req->stub = sp;
        omrpc_req_stub_info(rp->hp,sp->entry_name);

        /* wait */
        omrpc_request_wait_done(req);
        if(sp->stub_info == NULL)
            omrpc_fatal("omrpc_call_submit: stub NULL");
    }
    req = omrpc_new_req(rp,OMRPC_CALL);
    if(omrpc_tlog_flag) tlog_CALL_IN(rp->id);

    if(omrpc_tlog_flag) tlog_SEND_ARG_IN(rp->id);
    req->stub = sp;
    //    req->app = app;
    va_copy(req->app, *app);
    omrpc_setup_args(&(req->app),req->args,sp->stub_info);
    omrpc_call_send_args(rp->hp,sp->stub_info,req->args);
    if(omrpc_tlog_flag) tlog_SEND_ARG_OUT(rp->id);

    if(omrpc_debug_flag)
        omrpc_prf("omrpc_call_submit ... end rp=%p req=%p\n",rp,req);

    return req;
}

void omrpc_kill_rpc(omrpc_rpc_t *rp)
{

    if(omrpc_debug_flag) omrpc_prf("kill_rpc rp=%p\n",rp);
    omrpc_send_cmd(rp->hp,OMRPC_REQ_KILL);
    omrpc_send_done(rp->hp);
    omrpc_io_handle_close(rp->hp);
    omrpc_delete_rpc(rp);
}

/*
 * request data management:
 */
omrpc_request_t *omrpc_new_req(omrpc_rpc_t *rp,int req_kind)
{
    omrpc_request_t *req;

    RPC_LOCK;
    if(free_req == NULL){       /* allocate new */
        req = (omrpc_request_t *)omrpc_malloc(sizeof(omrpc_request_t));
        pthread_mutex_init(&req->lock,NULL);
        pthread_cond_init(&req->cond,NULL);
    } else {
        req = free_req;
        free_req = req->next;
    }
    RPC_UNLOCK;
    req->kind = req_kind;
    req->done_flag = FALSE;
    req->rp = rp;
    rp->req = req;
    return req;
}

int omrpc_request_wait_done(omrpc_request_t *req)
{
    if(omrpc_debug_flag) omrpc_prf("request_wait_done ... req=%p\n",req);
    /* wait on my lock */
    pthread_mutex_lock(&req->lock);
    while(!req->done_flag){
       pthread_cond_wait(&req->cond,&req->lock);
    }
    pthread_mutex_unlock(&req->lock);

    if(omrpc_debug_flag)
        omrpc_prf("request_wait_done ... get signal req=%p\n",req);

    RPC_LOCK;
#ifdef USE_FT
    pthread_mutex_lock(&omrpc_mpi_mutex);
    pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);
    if(req->rp && req->rp->mp && req->rp->mp->ft_status==OMRPC_MPI_DEAD){
      if(omrpc_debug_flag) omrpc_prf("request_wait_done find req[%p] is dead (module=%s)\n",req,req->rp->module->name);
      // free handles
      omrpcm_io_handle_close(&req->rp->mp);
      //omrpc_delete_rpc(req->rp);
      if(omrpc_tlog_flag) tlog_RPC_OUT(req->rp->id);
      req->rp->next = free_rpc_obj;
      free_rpc_obj = req->rp;
      free_req=req->next;
      req->rp=NULL;
      RPC_UNLOCK;
      pthread_mutex_unlock(&omrpc_mpi_mutex);
      return -1;
    }
    pthread_mutex_unlock(&omrpc_mpi_mutex);
#endif
    /* release this request structure */
    req->next = free_req;
    free_req = req;
    RPC_UNLOCK;

    if(omrpc_debug_flag) omrpc_prf("request_wait_done ... end req=%p\n",req);
#ifdef USE_FT
    /*
    if(req->rp->mp->ft_status==OMRPC_MPI_DEAD){
      return -1;
    }
    */
#endif
    return 0;
}


int omrpc_request_wait_any(int n, omrpc_request_t **req)
{
  int    i;
  int    flag;
  char   cmd;

  // * warning
  // temporary this implementation repeats probe for all reqs
  // until it find a finished request

  if(req == NULL)
    return -1;

  while(1){
    flag=INT_MIN;
    for(i=0;i<n;i++){
      RPC_LOCK;
      // i-th req is finished
      if(req[i]->done_flag) flag=i;
#ifdef USE_FT
      pthread_mutex_lock(&omrpc_mpi_mutex);
      pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);
      // i-th req is dead 
      if(req[i]->rp && req[i]->rp->mp && req[i]->rp->mp->ft_status==OMRPC_MPI_DEAD){ 
        if(omrpc_debug_flag) omrpc_prf("request_wait_any find req[%d] is dead\n",i);
#ifdef TEST_OMNIRPC_FT
        // kill this remote program manually since the OMRPC_MPI_DEAD is dummy for TEST
        if(omrpc_debug_flag) omrpc_prf("kill_rpc (TEST) rp=%p\n",req[i]->rp);
        cmd=OMRPC_REQ_KILL;
        omrpcm_send_char(req[i]->rp->mp, &cmd, 1, 0, req[i]->rp->mp->tag++);
#endif
        omrpcm_io_handle_close(&(req[i]->rp->mp));
        //omrpc_delete_rpc(req[i]->rp);
        if(omrpc_tlog_flag) tlog_RPC_OUT(req[i]->rp->id);
        req[i]->rp->next = free_rpc_obj;
        free_rpc_obj = req[i]->rp;
        free_req=req[i]->next;
        req[i]->rp=NULL;
        flag=-(i+1);
      }
      pthread_mutex_unlock(&omrpc_mpi_mutex);
#endif
      RPC_UNLOCK;
      if(flag!=INT_MIN) break; 
    }
    if(flag!=INT_MIN) break; 
  }
  return flag;

  /* : imifu.....
    // working
    if(omrpc_debug_flag) omrpc_prf("request_wait_any ...\n");
    omrpc_request_t *freq;

    RPC_LOCK;
    freq->next = free_req;
    free_req = freq;
    RPC_UNLOCK;
    if(omrpc_debug_flag) omrpc_prf("request_wait_done ... end req=%p\n",freq);
    return 0;
  */
}

/*
 * Handler thread: receive reply and execute lower-half
 */
void omrpc_rpc_handler(omrpc_request_t *req)
{
    omrpc_io_handle_t *hp;
    omrpc_rpc_t *rp;

    if(omrpc_debug_flag)
        omrpc_prf("handle request req=%p kind=%d(%s)\n",
                  req,req->kind,omrpc_req_kind_name(req->kind));
    rp = req->rp;
    hp = rp->hp;
    switch(req->kind){
    case OMRPC_INVOKE:  /* waiting startup */
        omrpc_io_handle_accept(hp);
        omrpc_recv_init_msg(hp,rp);
        omrpc_io_modified();    /* it may modify IO port */
        break;

    case OMRPC_STUB_INFO:
        omrpc_recv_stub_info(hp,&req->stub->stub_info);
        break;

    case OMRPC_CALL:
    case OMRPC_CALL_INIT:
        if(omrpc_tlog_flag) tlog_RECV_ARG_IN(rp->id);
        omrpc_call_recv_args(hp,req->stub->stub_info,req->args);
        if(omrpc_tlog_flag) tlog_RECV_ARG_OUT(rp->id);
        break;

    default:
        omrpc_fatal("omrpc_rpc_handler: bad state");
    }

    if(omrpc_tlog_flag){
        switch(req->kind){
        case OMRPC_INVOKE:     /* waiting startup */
            tlog_INVOKE_OUT(rp->id);
            break;
        case OMRPC_STUB_INFO:
            tlog_STUB_INFO_OUT(rp->id);
            break;
        case OMRPC_CALL:
            tlog_CALL_OUT(rp->id);
            break;
        case OMRPC_CALL_INIT:
            tlog_CALL_INIT_OUT(rp->id);
            break;
        }
    }

    /* release rpc object */
    RPC_LOCK;
    rp->req = NULL;     /* unbound */
    RPC_UNLOCK;

    if(req->kind == OMRPC_CALL && !rp->is_handle){
        omrpc_put_idle_rpc(rp);
    }

    /* notify waiting thread */
    pthread_mutex_lock(&req->lock);
    req->done_flag = TRUE;
    pthread_cond_signal(&req->cond);
    pthread_mutex_unlock(&req->lock);
}

static char *omrpc_req_kind_name(int kind)
{
    switch(kind){
    case OMRPC_INVOKE: return "INVOKE";
    case OMRPC_STUB_INFO: return "STUB_INFO";
    case OMRPC_CALL: return "CALL";
    case OMRPC_CALL_INIT: return "CALL_INIT";
    default: omrpc_prf("kind = %d\n",kind); return "????";
    }
} /* omrpc_req_kind_name */

#ifdef USE_FT
// return 1 (alive), 0 (error)
int omrpc_check_handle(omrpc_rpc_t *rp)
{
  int  ierr=1;
  char cmd;
  omrpc_request_t *req;

  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);
  RPC_LOCK;
  if(rp==NULL || rp->mp==NULL){
    ierr=0;
  }else if(rp->mp->ft_status==OMRPC_MPI_DEAD){
    ierr=0;
    if(omrpc_debug_flag) omrpc_prf("omrpc_check_handle finds handle is dead (module=%s)\n",rp->module->name);
#ifdef TEST_OMNIRPC_FT
     // kill this remote program manually since the OMRPC_MPI_DEAD is dummy for TEST
    if(omrpc_debug_flag) omrpc_prf("kill_rpc (TEST) rp=%p\n",rp);
    cmd=OMRPC_REQ_KILL;
    omrpcm_send_char(rp->mp, &cmd, 1, 0, rp->mp->tag++);
#endif
    // free handles
    omrpcm_io_handle_close(&rp->mp);
    if(omrpc_tlog_flag) tlog_RPC_OUT(rp->id);
    rp->next = free_rpc_obj;
    free_rpc_obj = rp;
  }
  RPC_UNLOCK;  
  pthread_mutex_unlock(&omrpc_mpi_mutex);

  return ierr;
} /* omrpc_check_handle */

// send signal to stop heart beat
int omrpcm_stop_handle(omrpc_rpc_t *rp)
{
  char cmd =OMRPC_STOP_HB;
  int  ierr=1;

  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);
  RPC_LOCK;

  omrpcm_send_char(rp->mp, &cmd, 1, 0, rp->mp->tag++);

  RPC_UNLOCK;
  pthread_mutex_unlock(&omrpc_mpi_mutex);

  return ierr;

}
#endif

