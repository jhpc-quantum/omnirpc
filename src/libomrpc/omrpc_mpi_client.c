/*
  M.TSUJI 2011.12
*/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "omrpc_host.h"
#include "omrpc_mpi.h"
#include "omrpc_mpi_io.h"
#include "omrpc_exec.h" 
#include "myx_master_wrapper.h"

omrpc_request_t *mfree_req = NULL;

extern pthread_cond_t  omrpc_mpi_cond;
extern pthread_mutex_t omrpc_mpi_mutex;

/***********************************************************************
 *  initialize staff in this module
 **********************************************************************/
void omrpcm_rpc_init()
{
    /* initialize mutex */
    //pthread_mutex_init(&mrpc_lock,NULL);
    //pthread_cond_init (&mrpc_wait,NULL);
    omrpcm_init_queue();

    if(omrpc_debug_flag) omrpc_prf("omrpc_rpc_init end ...\n");
} /* omrpcm_rpc_init */

void omrpcm_io_handle_close(omrpc_mpi_handle_t **mptr)
{
  omrpc_mpi_handle_t *mp;
  mp = *mptr;

  if(mpi_handle_head==mp){
    mpi_handle_head=mp->next;
  }
  if(mp->prev){
    mp->prev->next=mp->next;
  }
  if(mp->next){
    mp->next->prev=mp->prev;
  }

  if(mp) omrpc_free(mp);

  *mptr = NULL;
  return;
} /* omrpcm_io_handle_close */

/***********************************************************************
 * request data management:
 **********************************************************************/
omrpc_request_t *omrpcm_new_req(omrpc_rpc_t *rp,int req_kind)
{
    omrpc_request_t *req;

    if(mfree_req == NULL){       /* allocate new */
        req = (omrpc_request_t *)omrpc_malloc(sizeof(omrpc_request_t));
        pthread_mutex_init(&req->lock,NULL);
        pthread_cond_init(&req->cond,NULL);
    } else {
        req = mfree_req;
        mfree_req = req->next;
    }
    req->kind = req_kind;
    req->done_flag = FALSE;
    req->rp = rp;
    rp->req = req;
    return req;
} /* omrpcm_new_req */

int omrpcm_request_wait_done(omrpc_request_t *req, char is_sig_wait)
{
    if(omrpc_debug_flag) omrpc_prf("request_wait_done ... req=%p\n",req);

    if(is_sig_wait){
        pthread_mutex_lock(&req->lock);
        while(!req->done_flag){
            pthread_cond_wait(&req->cond,&req->lock);
        }
        pthread_mutex_unlock(&req->lock);
    }

    if(omrpc_debug_flag)
        omrpc_prf("request_wait_done ... get signal req=%p\n",req);

    //RPC_LOCK;
    // release this request structure 
    req->next = mfree_req;
    mfree_req = req;
    //RPC_UNLOCK;

    if(omrpc_debug_flag) omrpc_prf("request_wait_done ... end req=%p\n",req);

    return 0;

} /* omrpcm_request_wait_done */

/***********************************************************************
 * omrpcm_init_hosts
 * ... setup pseudo agent  omrpc_init_hosts()+omrpc_setup_agent()+omrpc_agent_read_registory()
 **********************************************************************/
void omrpcm_init_hosts()
{
  int    host_id;
  int    r, fd, off, len;
  char   buf[256],*registry_path;
  char   *cp;
  omrpc_host_t *hostp;

  host_id = 0;
  hostp = omrpc_hosts[host_id];
  mpi_handle_head = NULL;

  // registry_path will be NULL
  registry_path=buf;
  omrpc_get_default_path(buf,REGISTRY_FILE);

  if(omrpc_debug_flag){
    omrpc_prf("OMRPC_MPI_READ_REG: file='%s'\n",registry_path);
  }

  fd = open(registry_path,O_RDONLY);
  if(fd < 0){
    omrpc_prf("warning: cannot read registry on '%s'\n",
              omrpc_hosts[host_id]->name);
    return;
  }

  off = lseek(fd,0,SEEK_END);
  if(off < 0) omrpc_fatal("omrpc_send_file: lseek failed");
  if(off > INT_MAX) omrpc_fatal("omrpc_send_file: too large file");
  len = off;
  lseek(fd,0,SEEK_SET);

  cp = (char *)omrpc_malloc(len*sizeof(char));
  r = read(fd, cp, len);
  if(r!=len) omrpc_fatal("cannot read registory\n");

  omrpc_parse_registry(host_id,cp);
  omrpc_free(cp);
} /* omrpcm_init_hosts */

/***********************************************************************
 * omrpcm_spawn_module
 * ... spawn remote program
 **********************************************************************/
omrpc_rpc_t *omrpcm_spawn_module(omrpc_host_t *host,omrpc_module_t *module, int nprocs)
{
    int                pid;
    unsigned short     port;    
    short              magic, short_buf[6];
    double             et0;
    omrpc_rpc_t        *rp;
    omrpc_request_t    *req;
    omrpc_mpi_handle_t *mp;

    rp = omrpc_new_rpc();
    rp->host = host;
    rp->module = module;
    //agent_hp = host->agent_rp->hp;

    /* create request and activate */
    req = omrpcm_new_req(rp, OMRPC_INVOKE);
    if(omrpc_tlog_flag) tlog_INVOKE_IN(rp->id);

    rp->hp       = NULL;
    rp->mp       = omrpcm_handle_create();
    rp->mp->hint = (void *)rp;

    port = 0;
#ifdef USE_FT
    rp->mp->ft_fd=omrpc_io_socket(&port);
    MPI_Open_port(MPI_INFO_NULL,rp->mp->ft_pname);
    omrpc_prf("MPI_Open_port %s\n",rp->mp->ft_pname);
#endif
    if(omrpc_debug_flag)
      omrpc_prf("MPI_Comm_spawn module=%s nprocs=%d port=%u\n",module->name,nprocs,port);

    pid = omrpc_exec_by_mpi(omrpc_rex_prog_path(host,module),omrpc_my_hostname,port,host->working_path,nprocs,"MPI",&(rp->mp->comm));

#ifdef USE_FT
    // send port name for FJMPI_connect & accpet
    MPI_Send(rp->mp->ft_pname, MPI_MAX_PORT_NAME, MPI_CHAR, 0, 0, rp->mp->comm);
#endif

    if(pid < 0){
        perror("fork filed:");
        exit(1);
    }
    rp->mp->nprocs = nprocs;
    rp->mp->tag    = 0;

    if(mpi_handle_head==NULL){
      mpi_handle_head=rp->mp;
    }else{
      mp=mpi_handle_head;
      while(mp->next) mp=mp->next;
      mp->next=rp->mp;
      rp->mp->prev=mp;
    }

    omrpcm_recv_short(rp->mp, short_buf, 6, 0, rp->mp->tag++);

    magic         = short_buf[0];
    if(magic != OMRPC_STUB_MAGIC)
      omrpc_fatal("bad magic 0x%x != 0x%x",OMRPC_STUB_MAGIC,magic);
    rp->ver_major = short_buf[1];
    rp->ver_minor = short_buf[2];
    rp->have_init = short_buf[3];
    rp->n_entry   = short_buf[4];
    rp->pid       = short_buf[5];

#ifdef USE_FT
    // Establish communication for fault tolerant & get first ack from worker
    et0=MPI_Wtime();
    if(omrpc_debug_flag) omrpc_prf("init fault tolerant\n");
    rp->mp->ft_status=OMRPC_MPI_ALIVE;
    rp->mp->ft_cmfdt =MPI_Wtime();
    rp->mp->ft_fd=omrpc_io_accept(rp->mp->ft_fd);
    rp->mp->ft_fp=omrpc_io_handle_fd(rp->mp->ft_fd,FALSE);
    omrpc_io_handle_byte_order(rp->mp->ft_fp,TRUE);
    omrpc_recv_char(rp->mp->ft_fp,&(rp->mp->ft_status),1);
    omrpc_recv_done(rp->mp->ft_fp);
    if(omrpc_debug_flag) omrpc_prf("init fault tolerant ... done  %e(sec)\n",MPI_Wtime()-et0);
#endif
    
    if(omrpc_debug_flag) omrpc_prf("exec_module...invoke done\n");

    /* wait */
    rp->req        = NULL;
    req->done_flag = TRUE;
    omrpcm_request_wait_done(req, 0); 

    return rp;
} /* omrpcm_spawn_module */

/***********************************************************************
 * omrpcm_call_submit
 * ... send a request to remote program
 **********************************************************************/
omrpc_request_t *omrpcm_call_submit(omrpc_rpc_t *rp,
                                    omrpc_stub_entry_t *sp, va_list *app)
{
    omrpc_request_t *req;

    if(omrpc_debug_flag) omrpc_prf("omrpcm_call_submit ... rp=%p\n",rp);

    pthread_mutex_lock(&omrpc_mpi_mutex);
    pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);

    if(sp->stub_info == NULL){

        if(omrpc_debug_flag) omrpc_prf("omrpcm_call_submit ... req stub\n");
        req = omrpcm_new_req(rp,OMRPC_STUB_INFO);
        if(omrpc_tlog_flag) tlog_STUB_INFO_IN(rp->id);

        req->stub = sp;
        omrpcm_req_stub_info(rp->mp,sp->entry_name);
        omrpcm_recv_stub_info(rp->mp,&req->stub->stub_info);

        /* wait */
        req->rp->req   = NULL;
        req->done_flag = TRUE;
        omrpcm_request_wait_done(req, 0);

        if(sp->stub_info == NULL)
            omrpc_fatal("omrpcm_call_submit: stub NULL");
    }
    req = omrpcm_new_req(rp,OMRPC_CALL);
    if(omrpc_tlog_flag) tlog_CALL_IN(rp->id);

    if(omrpc_tlog_flag) tlog_SEND_ARG_IN(rp->id);
    req->stub = sp;
    //    req->app = app;
    va_copy(req->app, *app);
    omrpc_setup_args(&(req->app),req->args,sp->stub_info);
    omrpcm_call_send_args(rp->mp,sp->stub_info,req->args);
    if(omrpc_tlog_flag) tlog_SEND_ARG_OUT(rp->id);

    if(omrpc_debug_flag)
        omrpc_prf("omrpcm_call_submit ... end rp=%p req=%p\n",rp,req);

    pthread_mutex_unlock(&omrpc_mpi_mutex);

    return req;
} /* omrpcm_call_submit */

/***********************************************************************
 * omrpcm_schedule_rpc 
 *  ... simple round-robin scheduler. return rcp object.
 ***********************************************************************/
omrpc_rpc_t *omrpcm_schedule_rpc(omrpc_module_t *module, int nprocs)
{
    int id, status;
    int n_procs_killed, n_procs_req;
    omrpc_host_t *hp = NULL;
    omrpc_rpc_t *rp;

again:

    pthread_mutex_lock(&omrpc_mpi_mutex);
    pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);

    /* find IDLE rpc object */
    if((rp = omrpc_get_idle_rpc(module)) != NULL){
      pthread_mutex_unlock(&omrpc_mpi_mutex);
      return rp;
    }

    /* use empty nodes */
    hp = omrpc_hosts[0];
    if(hp->mpi_nodes_used+module->nprocs<=hp->mpi_nodes_total){
      rp = omrpcm_spawn_module(hp, module, nprocs);
      hp->mpi_nodes_used += module->nprocs;

      pthread_mutex_unlock(&omrpc_mpi_mutex);

      return rp;
    }

    /* if not found, find available host or candidate */
    id = 0; // there is only one host in the case using MPI_Comm_spawn ..
    if (omrpc_debug_flag) omrpc_prf("check host id = %d\n",id);
    hp = omrpc_hosts[id];
    if(!omrpc_host_has_module(id,module)){
      omrpc_fatal("module %s cannot be found\n",module->name);
    }
    status = omrpcm_allocate_jobs(id, nprocs);

    if(status){      /* sucess */
      //omrpc_set_last_sched_host(id);
      rp = omrpcm_spawn_module(hp, module, nprocs);
      pthread_mutex_unlock(&omrpc_mpi_mutex);

      return rp;
    }

    if(module->nprocs > hp->mpi_nodes_total-1){
      omrpc_fatal("# of required nodes %d for module %s is larger than total # of nodes %d\n",module->nprocs,module->name,hp->mpi_nodes_total);
    }

    n_procs_killed = 0;
    n_procs_req    = (hp->mpi_nodes_used+module->nprocs)-hp->mpi_nodes_total;
    while(n_procs_killed<n_procs_req || hp->n_jobs>=hp->max_jobs){
      if((rp = omrpcm_get_victim_idle_rpc(omrpc_hosts[id], module))!= NULL){
        if (omrpc_debug_flag)
          omrpc_prf("get victim idle rpc host id = %d (%s) , %d processes are killed\n",id,rp->module->name,rp->nprocs);
        n_procs_killed     += rp->nprocs;
        hp->mpi_nodes_used -= rp->nprocs;
        hp->n_jobs--;
        hp->max_jobs++;
        omrpcm_kill_rpc(rp);
        rp->nprocs=0;
      }else{
        break; // go to omrpc_wait_jobs();
      }
    } // while

    if(n_procs_killed>=n_procs_req && rp!=NULL){
      //omrpc_set_last_sched_host(candidate_host);
      rp = omrpcm_spawn_module(hp, module, nprocs);
      hp->mpi_nodes_used += module->nprocs;

      pthread_mutex_unlock(&omrpc_mpi_mutex);

      return rp;
    }

    pthread_mutex_unlock(&omrpc_mpi_mutex);

    omrpc_wait_jobs();

    goto again;
} /* omrpcm_schedule_rpc */

void omrpcm_kill_rpc(omrpc_rpc_t *rp)
{
  char cmd = OMRPC_REQ_KILL;

  if(omrpc_debug_flag) omrpc_prf("kill_rpc rp=%p (remote pid = %x)\n",rp,rp->pid);
  omrpcm_send_char(rp->mp, &cmd, 1, 0, rp->mp->tag++);
  omrpcm_io_handle_close(&(rp->mp));
  omrpc_delete_rpc(rp);
  if(omrpc_debug_flag) omrpc_prf("kill rpc .... killd\n");
} /* omrpcm_kill_rpc */

void omrpcm_finalize_rexes()
{
  char               cmd;
  omrpc_mpi_handle_t *mpi_ptr;
  omrpc_request_t    *req;
  omrpc_rpc_t        *rp;

  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);

  mpi_ptr = mpi_handle_head;
  while(mpi_ptr){
    rp  = (omrpc_rpc_t *)mpi_ptr->hint;
    req = rp->req;
    cmd       = OMRPC_REQ_KILL;
    if(omrpc_debug_flag) omrpc_prf("kill module_name=%s\n",rp->module->name);
    omrpcm_send_char(mpi_ptr,&cmd,1,0,mpi_ptr->tag++);
    mpi_ptr=mpi_ptr->next;
  }

  mpi_ptr = mpi_handle_head;
  while(mpi_ptr){
    rp  = (omrpc_rpc_t *)mpi_ptr->hint;
    req = rp->req;
    if(req)     omrpc_free(req);
    if(rp)      omrpc_free(rp);
#ifdef USE_FT
    if(mpi_ptr->ft_fp) omrpc_free(mpi_ptr->ft_fp);
#endif
    if(mpi_ptr->next){
      mpi_ptr=mpi_ptr->next;
      omrpc_free(mpi_ptr->prev);
    }else{
      // no ptr anymore
      omrpc_free(mpi_ptr);
      break; 
    }
  }

  mpi_handle_head=NULL;
  pthread_mutex_unlock(&omrpc_mpi_mutex);

} /* omrpc_finalize_rexes */

void omrpcm_prt_mpi()
{

  pthread_mutex_lock(&omrpc_mpi_mutex);
  pthread_cond_wait(&omrpc_mpi_cond,&omrpc_mpi_mutex);

  omrpc_mpi_handle_t *mpi_ptr = mpi_handle_head;
  while(mpi_ptr){
    omrpc_prf("mpi_handler=%p tag=%d\n",mpi_ptr,mpi_ptr->tag);
    mpi_ptr=mpi_ptr->next;
  }
  omrpc_prf("\n");
  pthread_mutex_unlock(&omrpc_mpi_mutex);

} /* omrpcm_prt_mpi */

