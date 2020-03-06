/* 20111129 M.TSUJI */

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "omrpc_host.h"
#include "omrpc_mpi.h"
#include "omrpc_mpi_io.h"

#define MPI_WORKING_PATH_DEFAULT "/tmp"

static pthread_mutex_t  mpi_lock;       /* general lock for omrpc_rpc */
static pthread_cond_t   mpi_cond; 

//static omrpc_rpc_t *free_rpc_obj = NULL;       /* free list */

volatile static char omrpcm_locked_flag = 0;
static int           omrpcm_mutex_tag = 0;

extern void ninfm_send_scalar_args(omrpc_mpi_handle_t *mp, NINF_STUB_INFO * sp,
                          any_t *args,int is_client);


/***********************************************************************
 * send args to start worker program
 ***********************************************************************/
int omrpcm_call_send_args(omrpc_mpi_handle_t *mp, NINF_STUB_INFO  *sp, any_t *args)
{
    int  i;
    char c;
    ninf_array_shape_info array_shape;
    struct ninf_param_desc *dp;

    c=OMRPC_REQ_CALL;
    omrpcm_send_char(mp, &c, 1, 0, mp->tag++);
    omrpcm_send_short(mp, &sp->index, 1, 0, mp->tag++);
    ninfm_send_scalar_args(mp,sp,args,TRUE);

    // receive vector data 
    for (i = 0; i < sp->nparam; i++){
        dp = &sp->params[i];
        if(dp->ndim == 0) continue;     // scalar 
        if(!IS_IN_MODE (dp->param_inout)) continue;
        // compute the number of elemnts 
        ninf_set_array_shape(dp,sp,args,&array_shape);
        ninfm_send_array(mp,args[i].p,&array_shape,0);
    }
    //omrpc_send_done(hp);
    return TRUE;
} /* omrpcm_call_send_args */

/***********************************************************************
 * recv results from rpc
 ***********************************************************************/
int omrpcm_call_recv_args(omrpc_mpi_handle_t *mp, NINF_STUB_INFO  *sp, any_t *args)
{
    char ack;
    int  i;
    ninf_array_shape_info array_shape;
    struct ninf_param_desc *dp;

    /* wait for return */
    omrpcm_recv_char(mp, &ack, 1, 0, OMRPC_END_STUB_TAG);

    for (i = 0; i < sp->nparam; i++){
        dp = &sp->params[i];
        //test
        //        if(dp->param_type == DT_FILEPOINTER)
        //            (dp->ndim)++;
        //end
        if(dp->ndim == 0) continue;     /* scalar */
        if(!IS_OUT_MODE (dp->param_inout)) continue;

        /* compute the number of elemnts */
        ninf_set_array_shape(dp,sp,args,&array_shape);
        ninfm_recv_array(mp,args[i].p,&array_shape,0);
    }
    //omrpc_recv_done(hp);
    return TRUE;
} /* omrpcm_call_recv_args */

char *omrpcm_req_kind_name(int kind)
{
    switch(kind){
    case OMRPC_INVOKE: return "INVOKE";
    case OMRPC_STUB_INFO: return "STUB_INFO";
    case OMRPC_CALL: return "CALL";
    case OMRPC_CALL_INIT: return "CALL_INIT";
    default: omrpc_prf("kind = %d\n",kind); return "????";
    }
} /* omrpcm_req_kind_name */

omrpc_mpi_handle_t *omrpcm_handle_create()
{
  omrpc_mpi_handle_t *mp;

  mp = (omrpc_mpi_handle_t *)omrpc_malloc(sizeof(omrpc_mpi_handle_t));
  mp->next = NULL;
  mp->prev = NULL;
  mp->hint = NULL;
  mp->nprocs =  0;
#ifdef USE_FT
  mp->ft_pname=(char *)malloc(sizeof(char)*MPI_MAX_PORT_NAME);
  //omrpc_prf("handle crated with ft_pname\n");
#endif

  return mp;
} /* omrpcm_handle_create */

