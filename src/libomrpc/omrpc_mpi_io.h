#ifndef _OMRPC_MPI_IO_H_
#define _OMRPC_MPI_IO_H_

#include "omrpc_stub.h"
#ifdef USE_FJMPI
#include "mpi-ext.h"
#endif

#define OMRPC_END_STUB_TAG  32767  // 32767=MAX_SHORT
#define OMRPC_ASKALIVE_TAG  32766

#ifdef USE_FT
#define OMRPC_MPI_ALIVE     31
#define OMRPC_MPI_DEAD      32
#define OMRPC_MPI_ABSENCE   33
#define OMRPC_MPI_HBINTVL   2.0
#endif

typedef struct omrpc_mpi_handle {
  int      nprocs;
  int      tag;
  struct   omrpc_mpi_handle *next, *prev;
  void     *hint;                 /* hint, used for store rpc object */
  MPI_Comm comm;                  /* inter-communicator */
#ifdef USE_FT
  int                ft_fd;        /* fd for ft */
  char               ft_status;    /* does this remote program dead or alive? */
  char               ft_send;      /* Must this remote program send HBs? */
  char               *ft_pname;    /* port name used in FJMPI_Mswk_connect-accept */
  double             ft_cmfdt;     /* the last time confirmed as alive        */
  omrpc_io_handle_t *ft_fp;        /* handle for fault tolerant */
#endif 
} omrpc_mpi_handle_t;

omrpc_mpi_handle_t *omrpcm_handle_create();

void omrpcm_send_int(omrpc_mpi_handle_t *mp, int *p, int n, int rank, int tag);
void omrpcm_recv_int(omrpc_mpi_handle_t *mp, int *p, int n, int rank, int tag);
void omrpcm_send_char(omrpc_mpi_handle_t *mp, char *p, int n, int rank, int tag);
void omrpcm_recv_char(omrpc_mpi_handle_t *mp, char *p, int n, int rank, int tag);
void omrpcm_send_str(omrpc_mpi_handle_t *mp, char *p, int rank, int tag);
void omrpcm_recv_str(omrpc_mpi_handle_t *mp, char *p, int rank, int tag);
void omrpcm_send_short(omrpc_mpi_handle_t *mp, short *p, int n, int rank, int tag);
void omrpcm_recv_short(omrpc_mpi_handle_t *mp, short *p, int n, int rank, int tag);
void omrpcm_send_long(omrpc_mpi_handle_t *mp, long *p, int n, int rank, int tag);
void omrpcm_recv_long(omrpc_mpi_handle_t *mp, long *p, int n, int rank, int tag);
void omrpcm_send_float(omrpc_mpi_handle_t *mp, float *p, int n, int rank, int tag);
void omrpcm_recv_float(omrpc_mpi_handle_t *mp, float *p, int n, int rank, int tag);
void omrpcm_send_double(omrpc_mpi_handle_t *mp, double *p, int n, int rank, int tag);
void omrpcm_recv_double(omrpc_mpi_handle_t *mp, double *p, int n, int rank, int tag);
void omrpcm_send_long_long(omrpc_mpi_handle_t *mp, long long *p, int n, int rank, int tag);
void omrpcm_recv_long_long(omrpc_mpi_handle_t *mp, long long *p, int n, int rank, int tag);
void omrpcm_send_double_complex(omrpc_mpi_handle_t *mp, double _Complex *p, int n, int rank, int tag);
void omrpcm_recv_double_complex(omrpc_mpi_handle_t *mp, double _Complex *p, int n, int rank, int tag);
void omrpcm_send_long_double(omrpc_mpi_handle_t *mp, long double *p, int n, int rank, int tag);
void omrpcm_recv_long_double(omrpc_mpi_handle_t *mp, long double *p, int n, int rank, int tag);
void omrpcm_send_float_complex(omrpc_mpi_handle_t *mp, float _Complex *p, int n, int rank, int tag);
void omrpcm_recv_float_complex(omrpc_mpi_handle_t *mp, float _Complex *p, int n, int rank, int tag);
void omrpcm_send_u_int(omrpc_mpi_handle_t *mp, unsigned int *p, int n, int rank, int tag);
void omrpcm_recv_u_int(omrpc_mpi_handle_t *mp, unsigned int *p, int n, int rank, int tag);
void omrpcm_send_u_char(omrpc_mpi_handle_t *mp, unsigned char *p, int n, int rank, int tag);
void omrpcm_recv_u_char(omrpc_mpi_handle_t *mp, unsigned char *p, int n, int rank, int tag);
void omrpcm_send_u_short(omrpc_mpi_handle_t *mp, unsigned short *p, int n, int rank, int tag);
void omrpcm_recv_u_short(omrpc_mpi_handle_t *mp, unsigned short *p, int n, int rank, int tag);
void omrpcm_send_u_long(omrpc_mpi_handle_t *mp, unsigned long *p, int n, int rank, int tag);
void omrpcm_recv_u_long(omrpc_mpi_handle_t *mp, unsigned long *p, int n, int rank, int tag);
void omrpcm_recv_strdup(omrpc_mpi_handle_t *p, char **cpp, int rank, int tag);
void omrpcm_send_u_long_long(omrpc_mpi_handle_t *mp, unsigned long long *p, int n, int rank, int tag);
void omrpcm_recv_u_long_long(omrpc_mpi_handle_t *mp, unsigned long long *p, int n, int rank, int tag);
void omrpcm_send_filename(omrpc_mpi_handle_t *mp, char **fname, int rank, int tag);
void omrpcm_recv_filename(omrpc_mpi_handle_t *mp, char **fname, int rank, int tag);
void omrpcm_send_filepointer(omrpc_mpi_handle_t *mp, FILE **fpp, int rank, int tag);
void omrpcm_recv_filepointer(omrpc_mpi_handle_t *mp, FILE **fpp, int rank, int tag);
void omrpcm_send_recv_char(omrpc_mpi_handle_t *p, char *dp,int count,int stride, int rw, int rank, int tag);
void omrpcm_send_recv_u_char(omrpc_mpi_handle_t *p, u_char *dp,int count,int stride, int rw, int rank, int tag);
void omrpcm_send_recv_short(omrpc_mpi_handle_t *p, short *dp,int count,int stride, int rw, int rank, int tag);
void omrpcm_send_recv_u_short(omrpc_mpi_handle_t *p, u_short *dp,int count,
                             int stride, int rw, int rank, int tag);
void omrpcm_send_recv_int(omrpc_mpi_handle_t *p,int *dp,int count,
                         int stride, int rw, int rank, int tag);
void omrpcm_send_recv_u_int(omrpc_mpi_handle_t *p, u_int *dp,int count,
                           int stride, int rw, int rank, int tag);
void omrpcm_send_recv_long(omrpc_mpi_handle_t *p, long *dp,int count,
                          int stride, int rw, int rank, int tag);
void omrpcm_send_recv_u_long(omrpc_mpi_handle_t *p, u_long *dp,int count,
                            int stride, int rw, int rank, int tag);
void omrpcm_send_recv_long_long(omrpc_mpi_handle_t *p, long long *dp,int count,
                               int stride, int rw, int rank, int tag);
void omrpcm_send_recv_u_long_long(omrpc_mpi_handle_t *p, unsigned long long *dp,
                                 int count,int stride, int rw, int rank, int tag);
void omrpcm_send_recv_float(omrpc_mpi_handle_t *p, float *dp,int count,
                           int stride, int rw, int rank, int tag);
void omrpcm_send_recv_double(omrpc_mpi_handle_t *p, double *dp,int count,
                            int stride, int rw, int rank, int tag);
void omrpcm_send_recv_long_double(omrpc_mpi_handle_t *p, long double *dp,int count,
                                 int stride, int rw, int rank, int tag);
void omrpcm_send_recv_float_complex(omrpc_mpi_handle_t *p, float _Complex *dp, int count,
                                   int stride, int rw, int rank, int tag);
void omrpcm_send_recv_double_complex(omrpc_mpi_handle_t *p, double _Complex *dp, int count,
                                    int stride, int rw, int rank, int tag);
void omrpcm_send_recv_string(omrpc_mpi_handle_t *p, char **dp,int count,
                            int stride, int rw, int rank, int tag);
void omrpcm_send_recv_filename(omrpc_mpi_handle_t *mp, char **dp, 
                               int count, int stride, int rw, int rank, int tag);
void omrpcm_send_recv_filepointer(omrpc_mpi_handle_t *mp, FILE **dp,
                                  int count, int stride, int rw, int rank, int tag);

void omrpcm_send_stub_info(omrpc_mpi_handle_t *mp, NINF_STUB_INFO  *sp);
int  omrpcm_recv_stub_info(omrpc_mpi_handle_t *mp, NINF_STUB_INFO  **spp);
int  omrpcm_req_stub_info(omrpc_mpi_handle_t *mp, char *name);
int  omrpcm_call_send_args(omrpc_mpi_handle_t *mp, NINF_STUB_INFO  *sp, any_t *args);

void ninfm_send_array(omrpc_mpi_handle_t *mp, char *base,
                      ninf_array_shape_info *ap, int rank);
void ninfm_recv_array(omrpc_mpi_handle_t *mp, char *base,
                      ninf_array_shape_info *ap, int rank);


#endif // _OMRPC_MPI_IO_H_

