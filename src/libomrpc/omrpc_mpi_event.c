/* 
  multi-thread event for OmniRPC-MPI 
  M.TSUJI 2011.12

  2013.05 FT 
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
#include "omrpc_mpi.h"
#include "omrpc_mpi_io.h"

#include "myx_master_wrapper.h"
static int to_fd,from_fd;

fd_set rfds;

int omrpc_handler_running;

// mutual exclusion for mpi_event_thread 
pthread_t       omrpc_mpi_tid;
pthread_attr_t  omrpc_mpi_t_attr;
pthread_cond_t  omrpc_mpi_cond     =PTHREAD_COND_INITIALIZER;
pthread_cond_t  omrpc_mpi_cond_init=PTHREAD_COND_INITIALIZER;
pthread_mutex_t omrpc_mpi_mutex    =PTHREAD_MUTEX_INITIALIZER;

extern int omrpcm_call_recv_args(omrpc_mpi_handle_t *mp, NINF_STUB_INFO  *sp, any_t *args);

void omrpcm_handle_rex_end(omrpc_mpi_handle_t *mp)
{
  omrpc_request_t *req;
  omrpc_rpc_t     *rp;
  rp  = (omrpc_rpc_t *)mp->hint;
  req = rp->req;

  //omprcm_debug_print_mp();

  if(omrpc_debug_flag)
    omrpc_prf("handle request end, mpi=%p req=%p module=%s pid=%p\n",
              mp,req,rp->module->name,rp->pid);
  switch(req->kind){
  case OMRPC_CALL:
    omrpcm_call_recv_args(mp,req->stub->stub_info,req->args);
    break;
  default:
    omrpc_fatal("omrpcm_handle_rex_end: bad state kind");
  }

  /* release rpc object */
  rp->req = NULL;     /* unbound */

  if(req->kind == OMRPC_CALL && !rp->is_handle){
    omrpc_put_idle_rpc(rp);
  } 
  /* notify waiting thread */
  pthread_mutex_lock(&req->lock);
  req->done_flag = TRUE;
  pthread_cond_signal(&req->cond);
  pthread_mutex_unlock(&req->lock);

  if(omrpc_debug_flag)
    omrpc_prf("handle request end, mpi=%p req=%p .......... end\n",
           mp,req);

  return;
} /* omrpcm_handle_rex_end */

#ifdef USE_FT
/* master returns worker "mpi_ptr" whether a worker specified by the worker is still alive or not */
void omrpcm_ask_alive(omrpc_mpi_handle_t *mpi_handle_head, omrpc_mpi_handle_t *mpi_ptr, int dest)
{
  short  rpid;
  char   status=OMRPC_MPI_ABSENCE;
  omrpc_rpc_t        *rp;
  omrpc_mpi_handle_t *ptr;

  omrpcm_recv_short(mpi_ptr,&rpid,1,dest,OMRPC_ASKALIVE_TAG);
  ptr=mpi_handle_head;
  while(ptr){
    rp=(omrpc_rpc_t *)ptr->hint;
    if(rpid==rp->id){
      status=ptr->ft_status;
      break;
    }
    ptr=ptr->next;
  }
  // 確認せずにおくる
  MPI_Send(&status,1,MPI_CHAR,dest,OMRPC_ASKALIVE_TAG,mpi_ptr->comm);
}
#endif

/* hander thread main (handle the err & ending of remote programs) */
static void *omrpcm_handler_main(void *dummy)
{
  int    ierr,flag = 0;
  MPI_Status         status;
  omrpc_rpc_t        *rp;
  omrpc_mpi_handle_t *mpi_ptr;
#ifdef USE_FT
  int    max_nfd,nfd,fd;
  fd_set rfds;
  struct timeval     tm;
  double et;
#endif

 wait_new:
  while(mpi_handle_head==NULL){
    pthread_mutex_lock(&omrpc_mpi_mutex);
    sched_yield();
    pthread_cond_signal(&omrpc_mpi_cond);
    pthread_mutex_unlock(&omrpc_mpi_mutex);
  }
 again:

  pthread_mutex_lock(&omrpc_mpi_mutex);

  if(!mpi_handle_head){
    pthread_cond_signal(&omrpc_mpi_cond);
    pthread_mutex_unlock(&omrpc_mpi_mutex);
    
    sched_yield();

    goto wait_new;
  }

  mpi_ptr=mpi_handle_head;
  while(mpi_ptr){
    rp = (omrpc_rpc_t *)mpi_ptr->hint;
#ifdef USE_FT
    if(mpi_ptr->ft_status!=OMRPC_MPI_ALIVE){
      mpi_ptr=mpi_ptr->next;
      continue;
    }
#endif
    if(rp && rp->nprocs>0){
      // recieve the signal to end
      flag=0;
      ierr=MPI_Iprobe(0,OMRPC_END_STUB_TAG,mpi_ptr->comm,&flag,&status);
      if(flag){
	omrpcm_handle_rex_end(mpi_ptr);
	break;
      }
#ifdef USE_FT
      // recieve ask alive 
      flag=0;
      ierr=MPI_Iprobe(MPI_ANY_SOURCE,OMRPC_ASKALIVE_TAG,mpi_ptr->comm,&flag,&status);
      if(flag){
        omrpcm_ask_alive(mpi_handle_head, mpi_ptr, status.MPI_SOURCE);
      }
      if(ierr!=MPI_SUCCESS){
	if(omrpc_debug_flag) omrpc_prf("ERROR? : MPI_ERROR in \"%s\" ierr=%d\n",rp->module->name,ierr);
	mpi_ptr->ft_status=OMRPC_MPI_DEAD;
	break;
      }
#endif
    }
    mpi_ptr=mpi_ptr->next;
  }
  pthread_cond_signal(&omrpc_mpi_cond);
  pthread_mutex_unlock(&omrpc_mpi_mutex);

  sched_yield();

#ifdef USE_FT
  // wait any heart beat signals
  FD_ZERO(&rfds);
  mpi_ptr=mpi_handle_head;
  max_nfd=mpi_ptr->ft_fd;
  while(mpi_ptr){
    fd=mpi_ptr->ft_fd;
    if(fd>max_nfd) max_nfd=fd;
    FD_SET(fd,&rfds);
    mpi_ptr=mpi_ptr->next;
  }

  sched_yield();

 retry:
  //tm.tv_sec =TIMEOUT_SEC;
  tm.tv_sec = 0;
  tm.tv_usec= 0;
  nfd=select(max_nfd+1,&rfds,NULL,NULL,&tm);
  if(nfd<0){
    if(errno==EINTR){
      goto retry;
    }
    perror("select");
    omrpc_fatal("handler select failed");
  }else if(nfd>0){
    mpi_ptr=mpi_handle_head;
    while(mpi_ptr){
      fd=mpi_ptr->ft_fd;
      rp=(omrpc_rpc_t *)mpi_ptr->hint;
      if(mpi_ptr->ft_status!=OMRPC_MPI_DEAD && FD_ISSET(fd,&rfds)){
	omrpc_recv_char(mpi_ptr->ft_fp,&(mpi_ptr->ft_status),1);
	omrpc_recv_done(mpi_ptr->ft_fp);
	if(mpi_ptr->ft_status==OMRPC_MPI_ALIVE){ 
	  //if(omrpc_debug_flag) omrpc_prf("module=%s id=%d is alive\n",rp->module->name,rp->id);
	  mpi_ptr->ft_cmfdt=MPI_Wtime();
        }
      }
      mpi_ptr=mpi_ptr->next;
    }
  }
  // check the time of the latest heart beat
  et=MPI_Wtime(); 
  mpi_ptr=mpi_handle_head;
  while(mpi_ptr){
    rp=(omrpc_rpc_t *)mpi_ptr->hint;
    if((mpi_ptr->ft_status==OMRPC_MPI_ALIVE) && 
       et-mpi_ptr->ft_cmfdt>(OMRPC_MPI_HBINTVL*2)){
      mpi_ptr->ft_status=OMRPC_MPI_DEAD;
      omrpc_prf("ERROR: module=%s id=%d would be dead. The last heartbeat was received %f sec before\n",rp->module->name,rp->id,et-mpi_ptr->ft_cmfdt);
    }
    mpi_ptr=mpi_ptr->next;
  }
#endif
 // pthread_cond_signal(&omrpc_mpi_cond);
 // pthread_mutex_unlock(&omrpc_mpi_mutex);

  sched_yield();
  
  goto again; /* loop forever */
  
  return NULL;
} /* omrpcm_handler_main */

void omrpcm_io_event_init()
{
    int fds[2];
    int r;

    if(pipe(fds) != 0){
        perror("pipe");
        omrpc_fatal("cannot create pipe");
    }
    // communication pipe between caller to handler 
    to_fd = fds[1];
    from_fd = fds[0];

    // create thread 
    pthread_attr_init(&omrpc_mpi_t_attr);
    r=pthread_create(&omrpc_mpi_tid,&omrpc_mpi_t_attr,omrpcm_handler_main, NULL);

    if(r!=0){
        omrpc_fatal("cannot create handler thread");
    }
    omrpc_handler_running = TRUE;
    
    if(omrpc_debug_flag) omrpc_prf("omrpc_io_event_init end ...\n");
} /* omrpcm_io_event_init */


