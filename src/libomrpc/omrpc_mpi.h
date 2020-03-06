#ifndef _OMPRC_MPI_H_
#define _OMRPC_MPI_H_

#include <mpi.h>
#include <pthread.h>
#include "omrpc_rpc.h"

pthread_t mrpc_tid;

omrpc_mpi_handle_t *mpi_handle_head;

int  omrpcm_allocate_jobs(int host_id, int nprocs); // (omrpc_schedule.c)
int  omrpcm_request_wait_done(omrpc_request_t *req, char is_sig_wait);
void omrpcm_init_hosts(void);
void omrpcm_io_event_init(void);
void omrpcm_kill_rpc(omrpc_rpc_t *rp);
void omrpcm_rpc_init(void);
char *omrpcm_req_kind_name(int kind);


// the following functions are written in omrpc_mpi_client.c
omrpc_rpc_t     *omrpcm_spawn_module(omrpc_host_t *host,omrpc_module_t *module, int nprocs); 
omrpc_rpc_t     *omrpcm_schedule_rpc(omrpc_module_t *module, int nprocs);
omrpc_request_t *omrpcm_call_submit(omrpc_rpc_t *rp,
                                    omrpc_stub_entry_t *sp, va_list *app);
void             omrpcm_finalize_rexes();

#endif // _OMRPC_MPI_H_
