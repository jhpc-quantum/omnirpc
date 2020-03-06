#include "omrpc_mpi_io.h"
#include "myx_master_wrapper.h"

extern int omrpc_get_fp_size(FILE *fp);

#ifdef USE_FT
char fjmpi_is_master=1;

#ifndef USE_FJMPI
/*
int FJMPI_Mswk_accept(char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm)
{
  int ret;
  ret = MPI_Comm_accept(port_name,info,root,comm,newcomm);
  return ret;
}

int FJMPI_Mswk_connect(char *port_name, MPI_Info info, int root, MPI_Comm comm, MPI_Comm *newcomm)
{
  int ret;
  ret = MPI_Comm_connect(port_name,info,root,comm,newcomm);
  return ret;
}

int FJMPI_Mswk_disconnect(MPI_Comm *comm)
{
  int ret;
  ret = MPI_Comm_disconnect(comm);
  return ret;
}
*/
#endif

int FJMPI_Mswk_send(void *buf, int count, MPI_Datatype datatype, int dest,
		    int tag, char *port, MPI_Comm comm){

  MPI_Comm newcomm;
  int ret;

  if(fjmpi_is_master){
    ret = FJMPI_Mswk_accept (port, MPI_INFO_NULL, 0, MPI_COMM_SELF, &newcomm);

    if (ret != MPI_SUCCESS) return ret;

    ret = MPI_Send(buf, count, datatype, dest, tag, newcomm);
    if (ret != MPI_SUCCESS) return ret;

    ret = FJMPI_Mswk_disconnect(&newcomm);
    if (ret != MPI_SUCCESS) return ret;
  }else{
    ret = MPI_Send(buf, count, datatype, dest, tag, comm);
    if (ret != MPI_SUCCESS) return ret;
  }

  return MPI_SUCCESS;

} /* FJMPI_Mswk_send */

int FJMPI_Mswk_recv(void *buf, int count, MPI_Datatype datatype, int source,
		    int tag, char *port, MPI_Status *status, MPI_Comm comm){

  MPI_Comm newcomm;
  int ret;

  if(!fjmpi_is_master){
    ret = FJMPI_Mswk_connect(port, MPI_INFO_NULL, 0, MPI_COMM_SELF, &newcomm);

    if (ret != MPI_SUCCESS) return ret;

    ret = MPI_Recv(buf, count, datatype, source, tag, newcomm, status);
    if (ret != MPI_SUCCESS) return ret;
  
    ret = FJMPI_Mswk_disconnect(&newcomm);
    if (ret != MPI_SUCCESS) return ret;
  }else{
    ret = MPI_Recv(buf, count, datatype, source, tag, comm, status);
    if (ret != MPI_SUCCESS) return ret;
  }

  return MPI_SUCCESS;

} /* FJMPI_Mswk_recv */
#endif

void omrpcm_send_int(omrpc_mpi_handle_t *mp, int *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_INT, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_INT, rank, tag, mp->comm);
#endif
} /* omrpcm_send_int */

void omrpcm_recv_int(omrpc_mpi_handle_t *mp, int *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_INT, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_INT, rank, tag, mp->comm, &status);
#endif
} /* omrpcm_recv_int */

void omrpcm_send_u_int(omrpc_mpi_handle_t *mp, unsigned int *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_UNSIGNED, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_UNSIGNED, rank, tag, mp->comm);
#endif
} /* omrpcm_send_u_int */

void omrpcm_recv_u_int(omrpc_mpi_handle_t *mp, unsigned int *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_UNSIGNED, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_UNSIGNED, rank, tag, mp->comm, &status);
#endif
} /* omrpcm_recv_u_int */

void omrpcm_send_char(omrpc_mpi_handle_t *mp, char *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_CHAR, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_CHAR, rank, tag, mp->comm);
#endif
} /* omrpcm_send_char */

void omrpcm_recv_char(omrpc_mpi_handle_t *mp, char *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_CHAR, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_CHAR, rank, tag, mp->comm, &status);
#endif
} /* omrpcm_recv_char */

void omrpcm_send_u_char(omrpc_mpi_handle_t *mp, unsigned char *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_UNSIGNED_CHAR, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_UNSIGNED_CHAR, rank, tag, mp->comm);
#endif
} /* omrpcm_send_u_char */

void omrpcm_recv_u_char(omrpc_mpi_handle_t *mp, unsigned char *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_UNSIGNED_CHAR, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_UNSIGNED_CHAR, rank, tag, mp->comm, &status);
#endif
} /* omrpcm_recv_u_char */

void omrpcm_send_str(omrpc_mpi_handle_t *mp, char *p, int rank, int tag)
{
    int l;

    l=strlen(p)+1;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(&l, 1, MPI_INT, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(&l, 1, MPI_INT,   rank, tag,   mp->comm);
#endif
#ifdef USE_FJMPI
    ierr=FJMPI_Mswk_send(p, l, MPI_CHAR, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p,  l, MPI_CHAR, rank, tag+1, mp->comm);
#endif
} /* omrpcm_send_str */

void omrpcm_recv_str(omrpc_mpi_handle_t *mp, char *p, int rank, int tag)
{
    int        l;
    MPI_Status status;

#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(&l, 1, MPI_INT, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else   
    MPI_Recv(&l, 1, MPI_INT,   rank, tag,   mp->comm, &status);
#endif
#ifdef USE_FJMPI
    ierr=FJMPI_Mswk_recv(p, l, MPI_CHAR, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, l,  MPI_CHAR, rank, tag+1, mp->comm, &status);
#endif
} /* omrpcm_recv_str */

void omrpcm_send_short(omrpc_mpi_handle_t *mp, short *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_SHORT, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_SHORT, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_short */

void omrpcm_recv_short(omrpc_mpi_handle_t *mp, short *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_SHORT, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_SHORT, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_short */

void omrpcm_send_u_short(omrpc_mpi_handle_t *mp, unsigned short *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_UNSIGNED_SHORT, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_UNSIGNED_SHORT, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_u_short */

void omrpcm_recv_u_short(omrpc_mpi_handle_t *mp, unsigned short *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_UNSIGNED_SHORT, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_UNSIGNED_SHORT, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_u_short */

void omrpcm_send_long(omrpc_mpi_handle_t *mp, long *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_LONG, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_LONG, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_long */

void omrpcm_recv_long(omrpc_mpi_handle_t *mp, long *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_LONG, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_LONG, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_long */

void omrpcm_send_u_long(omrpc_mpi_handle_t *mp, unsigned long *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_UNSIGNED_LONG, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_UNSIGNED_LONG, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_u_long */

void omrpcm_recv_u_long(omrpc_mpi_handle_t *mp, unsigned long *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_UNSIGNED_LONG, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_UNSIGNED_LONG, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_u_long */

void omrpcm_send_float(omrpc_mpi_handle_t *mp, float *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_FLOAT, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_FLOAT, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_float */

void omrpcm_recv_float(omrpc_mpi_handle_t *mp, float *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_FLOAT, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_FLOAT, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_float */

void omrpcm_send_double(omrpc_mpi_handle_t *mp, double *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_DOUBLE, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_DOUBLE, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_double */

void omrpcm_recv_double(omrpc_mpi_handle_t *mp, double *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_DOUBLE, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_DOUBLE, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_double */

void omrpcm_send_long_long(omrpc_mpi_handle_t *mp, long long *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_LONG_LONG_INT, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_LONG_LONG_INT, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_long_long */

void omrpcm_recv_long_long(omrpc_mpi_handle_t *mp, long long *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_LONG_LONG_INT, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_LONG_LONG_INT, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_long_long */

void omrpcm_send_u_long_long(omrpc_mpi_handle_t *mp, unsigned long long *p, int n, int rank, int tag)
{
    omrpcm_send_long_long(mp, p, n, rank, tag);
} /*  omrpcm_send_u_long_long */

void omrpcm_recv_u_long_long(omrpc_mpi_handle_t *mp, unsigned long long *p, int n, int rank, int tag)
{
    omrpcm_recv_long_long(mp, p, n, rank, tag);
} /*  omrpcm_recv_u_long_long */

void omrpcm_send_float_complex(omrpc_mpi_handle_t *mp, float _Complex *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_COMPLEX, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_COMPLEX, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_float_complex */

void omrpcm_recv_float_complex(omrpc_mpi_handle_t *mp, float _Complex *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_COMPLEX, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_COMPLEX, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_float_complex */

void omrpcm_send_double_complex(omrpc_mpi_handle_t *mp, double _Complex *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_DOUBLE_COMPLEX, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_DOUBLE_COMPLEX, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_double_complex */

void omrpcm_recv_double_complex(omrpc_mpi_handle_t *mp, double _Complex *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_DOUBLE_COMPLEX, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_DOUBLE_COMPLEX, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_double_complex */

void omrpcm_send_long_double(omrpc_mpi_handle_t *mp, long double *p, int n, int rank, int tag)
{
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_send(p, n, MPI_LONG_DOUBLE, rank, tag, mp->ft_pname, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Send(p, n, MPI_LONG_DOUBLE, rank, tag, mp->comm);
#endif
} /*  omrpcm_send_long_double */

void omrpcm_recv_long_double(omrpc_mpi_handle_t *mp, long double *p, int n, int rank, int tag)
{
    MPI_Status status;
#ifdef USE_FJMPI
    int ierr;
    ierr=FJMPI_Mswk_recv(p, n, MPI_LONG_DOUBLE, rank, tag, mp->ft_pname, &status, mp->comm);
    if(ierr!=MPI_SUCCESS) mp->ft_status=OMRPC_MPI_DEAD;
#else
    MPI_Recv(p, n, MPI_LONG_DOUBLE, rank, tag, mp->comm, &status);
#endif
} /*  omrpcm_recv_long_double */

void omrpcm_recv_strdup(omrpc_mpi_handle_t *p,char **cpp, int rank, int tag)
{
    int len;
    char *cp;
    omrpcm_recv_int(p,&len,1,rank,tag);
    if(len != 0) {
        cp = (char *)omrpc_malloc(len+1);
        omrpcm_recv_char(p,cp,len,rank,tag+1);
        *cpp = cp;
    } else
        *cpp = NULL;
} /* omrpcm_recv_strdup */

void omrpcm_send_filename(omrpc_mpi_handle_t *mp, char **fname, int rank, int tag)
{
    int  len;
    int  r;
    char *ctmp;
    FILE *tmpfp;

    /* pre-procedure */
    if(omrpc_is_client){
        tmpfp = fopen(*fname, "r");
        if(tmpfp == NULL){
            omrpc_error("omrpc_send_filepointer: cannot open: %s\n", *fname);
        }
    } else {
        tmpfp = fopen(*fname, "r");
        if(tmpfp == NULL){
            omrpc_error("omrpc_send_filepointer: cannot open: %s\n", *fname);
        }
    }

    if (tmpfp == NULL){
        len = -1;  // inform the pointer was NULL 
        omrpcm_send_int(mp, &len, 1, rank, tag);
        return;
    }

    if ((len = omrpc_get_fp_size(tmpfp)) < 0){
        omrpc_error("omrpc_send_filepointer: cannot get file size\n");
    }

    // send file size 
    omrpcm_send_int(mp, &len, 1, rank, tag);

    // read file
    ctmp = (char *)omrpc_malloc(sizeof(char)*(len+1));
    r = fread(ctmp, 1, len, tmpfp);
    if(r != len) omrpc_fatal("omrpc_send_file: read fail r=%d",r);

    // send file
    omrpcm_send_char(mp, ctmp, len, 0, 0);

    omrpc_free(ctmp);

    /* post proceeding */
    if(omrpc_is_client){
        fflush(tmpfp);
        fclose(tmpfp);
    } else {
        fflush(tmpfp);
        fclose(tmpfp);
    }
} /* omrpcm_send_filename */

void omrpcm_recv_filename(omrpc_mpi_handle_t *mp, char **fname, int rank, int tag)
{
    FILE *tmpfp;
    int  len;
    int  r;
    char *ctmp;

    /* pre-procedure  */
    if(omrpc_is_client){
        if(*fname == NULL){
            tmpfp = omrpc_make_tmp_fp_filename("w+", fname);
            if(tmpfp == NULL){
                omrpc_error("omrpc_recv_filename: cannot open file");
            }
        } else {
            tmpfp = fopen(*fname, "w+");
        }
    } else {
        tmpfp = omrpc_make_tmp_fp_filename("w+", fname);
        if(tmpfp == NULL){
            omrpc_error("omrpc_recv_filename: cannot open file");
        }
    }

    if(omrpc_debug_flag){
        omrpc_prf("temporary filename : %s\n", *fname);
    }

    /* get file length */
    omrpcm_recv_int(mp, &len, 1, rank, tag);
    if (omrpc_debug_flag){
        omrpc_prf("omrpc_recv_filepoiner: filesize=%d\n", len);
    }

    /* the filepointer / finename was null */
    if (len == -1){
        tmpfp = NULL;
        return;
    }

    /* write out current buffer
     * and send file to other side host
     */
    ctmp = (char *)omrpc_malloc(sizeof(char)*len);
    omrpcm_recv_char(mp, ctmp, len, rank, tag);
    r    = fwrite(ctmp, 1, len, tmpfp);
    omrpc_free(ctmp);

    /* post proceeding */
    if(omrpc_is_client){
        fflush(tmpfp);
        fclose(tmpfp);
    } else {
        fflush(tmpfp);
        fclose(tmpfp);
    }
} /* omrpcm_recv_filename */

void omrpcm_send_filepointer(omrpc_mpi_handle_t *mp, FILE **fpp, int rank, int tag)
{
    int  len;
    int  nbytes;
    int  r;
    char *ctmp;

    /* flush the file buffer */
    rewind(*fpp);
    fflush(*fpp);

    if (*fpp == NULL){
        len = -1;  /* inform the pointer was NULL */
        omrpcm_send_int(mp, &len, 1, 0, tag);
        return;
    }

    if ((len = omrpc_get_fp_size(*fpp)) < 0){
        omrpc_error("omrpc_send_filepointer: cannot get file size\n");
    }

    /* send the size of file */
    omrpcm_send_int(mp, &len, 1, 0, tag);
    ctmp = (char *)omrpc_malloc((len+1)*sizeof(char));
    r = fread(ctmp, 1, len, *fpp);
    omrpcm_send_char(mp, ctmp, len, 0, tag);
    omrpc_free(ctmp);    

    /* post proceeding */
    if(omrpc_is_client){
        ; /* nothing to do */
    } else {
        fclose(*fpp);
    }
} /* omrpcm_send_filepointer */

void omrpcm_recv_filepointer(omrpc_mpi_handle_t *mp, FILE **fpp, int rank, int tag)
{
    FILE *tmpfp;
    int  len, r;
    char *ctmp;

    /* pre-procedure  */
    if(omrpc_is_client){
        ;
    } else {
        /* create new filepointer */
        tmpfp = omrpc_make_tmp_fp("w+");
        if (tmpfp == NULL){
            omrpc_error("omrpc_recv_filepointer: make_tmp_fp\n");
        }
        *fpp = (FILE *)tmpfp;
    }

    /* get file length */
    omrpcm_recv_int(mp, &len, 1, 0, tag);
    if (omrpc_debug_flag){
        omrpc_prf("omrpc_recv_filepoiner: filesize = %d\n", len);
    }

    /* the filepointer / finename was null */
    if (len == -1){
        *fpp = NULL;
        return;
    }

    /* write out current buffer 
     * and send file to other side host
     */
    ctmp = (char *)omrpc_malloc((len+1)*sizeof(char));
    omrpcm_send_char(mp, ctmp, len, 0, tag);
    r = fwrite(ctmp, 1, len, *fpp);
    omrpc_free(ctmp);

    /* post proceeding */
    fflush(*fpp);
    rewind(*fpp);
} /* omrpcm_recv_filepointer */

void omrpcm_send_recv_char(omrpc_mpi_handle_t *p, char *dp,int count,
                          int stride, int rw, int rank, int tag)
{
    if(stride == 1 || stride == 0){
        if(rw) omrpcm_recv_char(p,dp,count,rank,tag);
        else omrpcm_send_char(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw){
                omrpcm_recv_char(p,dp,1,rank,tag);
            } else{
                omrpcm_send_char(p,dp,1,rank,tag);
            }
            dp += stride;
        }
    }
} /* omrpcm_send_recv_char */

void omrpcm_send_recv_u_char(omrpc_mpi_handle_t *p, u_char *dp,int count,int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_u_char(p,dp,count,tag,rank);
        else omrpcm_send_u_char(p,dp,count,tag,rank);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_u_char(p,dp,1,tag,rank);
            else omrpcm_send_u_char(p,dp,1,tag,rank);
            dp += stride;
        }
    }
} /* omrpcm_send_recv_u_char */

void omrpcm_send_recv_short(omrpc_mpi_handle_t *p, short *dp,int count,int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_short(p,dp,count,rank,tag);
        else omrpcm_send_short(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_short(p,dp,1,rank,tag);
            else omrpcm_send_short(p,dp,1,rank,tag);
            dp += stride;
        }
    }
} /* omrpcm_send_recv_short */

void omrpcm_send_recv_u_short(omrpc_mpi_handle_t *p, u_short *dp,int count,
                             int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_u_short(p,dp,count,rank,tag);
        else omrpcm_send_u_short(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_u_short(p,dp,1,rank,tag);
            else omrpcm_send_u_short(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_int(omrpc_mpi_handle_t *p,int *dp,int count,
                         int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_int(p,dp,count,rank,tag);
        else omrpcm_send_int(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_int(p,dp,1,rank,tag);
            else omrpcm_send_int(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_u_int(omrpc_mpi_handle_t *p, u_int *dp,int count,
                           int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_u_int(p,dp,count,rank,tag);
        else omrpcm_send_u_int(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_u_int(p,dp,1,rank,tag);
            else omrpcm_send_u_int(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_long(omrpc_mpi_handle_t *p, long *dp,int count,
                          int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_long(p,dp,count,rank,tag);
        else omrpcm_send_long(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_long(p,dp,1,rank,tag);
            else omrpcm_send_long(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_u_long(omrpc_mpi_handle_t *p, u_long *dp,int count,
                            int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_u_long(p,dp,count,rank,tag);
        else omrpcm_send_u_long(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_u_long(p,dp,1,rank,tag);
            else omrpcm_send_u_long(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_long_long(omrpc_mpi_handle_t *p, long long *dp,int count,
                               int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_long_long(p,dp,count,rank,tag);
        else omrpcm_send_long_long(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_long_long(p,dp,1,rank,tag);
            else omrpcm_send_long_long(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_u_long_long(omrpc_mpi_handle_t *p, unsigned long long *dp,
                                 int count,int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_u_long_long(p,dp,count,rank,tag);
        else omrpcm_send_u_long_long(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_u_long_long(p,dp,1,rank,tag);
            else omrpcm_send_u_long_long(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_float(omrpc_mpi_handle_t *p, float *dp,int count,
                           int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_float(p,dp,count,rank,tag);
        else omrpcm_send_float(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_float(p,dp,1,rank,tag);
            else omrpcm_send_float(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_double(omrpc_mpi_handle_t *p, double *dp,int count,
                            int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_double(p,dp,count,rank,tag);
        else omrpcm_send_double(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_double(p,dp,1,rank,tag);
            else omrpcm_send_double(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_long_double(omrpc_mpi_handle_t *p, long double *dp,int count,
                                 int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_long_double(p,dp,count,rank,tag);
        else omrpcm_send_long_double(p,dp,count,rank,tag);
    } else {
        while(count-- > 0){
            if(rw) omrpcm_recv_long_double(p,dp,1,rank,tag);
            else omrpcm_send_long_double(p,dp,1,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_float_complex(omrpc_mpi_handle_t *p, float _Complex *dp, int count,
                                   int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_float_complex(p,dp,count,rank,tag);
        else omrpcm_send_float_complex(p,dp,count,rank,tag);
    } else {
        while(count -- > 0){
            if(rw) omrpcm_recv_float_complex(p,dp,count,rank,tag);
            else omrpcm_send_float_complex(p,dp,count,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_double_complex(omrpc_mpi_handle_t *p, double _Complex *dp, int count,
                                    int stride, int rw, int rank, int tag)
{
    if(stride == 0 || stride == 1){
        if(rw) omrpcm_recv_double_complex(p,dp,count,rank,tag);
        else omrpcm_send_double_complex(p,dp,count,rank,tag);
    } else {
        while(count -- > 0){
            if(rw) omrpcm_recv_double_complex(p,dp,count,rank,tag);
            else omrpcm_send_double_complex(p,dp,count,rank,tag);
            dp += stride;
        }
    }
}

void omrpcm_send_recv_string(omrpc_mpi_handle_t *p, char **dp,int count,
                            int stride, int rw, int rank, int tag)
{
    int i;
    if(stride <= 1){
        if(rw){ /* recv */
            for(i = 0; i < count; i++) omrpcm_recv_strdup(p, dp+i,rank,tag);
        } else {
            for(i = 0; i < count; i++) omrpcm_send_str(p, dp[i],rank,tag);
        }
    } else
        omrpc_fatal("no stride (%d) is supported in string send/recv",
                    stride);
}

void omrpcm_send_recv_filename(omrpc_mpi_handle_t *mp, char **dp, 
                               int count, int stride, int rw, int rank, int tag)
{
    int i;
    if(stride <= 1){
        if(rw) { /* recv */
            for(i = 0; i < count; i++){
                omrpcm_recv_filename(mp, &(dp[i]), rank, tag);
            }
        } else {
            for(i = 0; i < count; i++){
                omrpcm_send_filename(mp, &(dp[i]), rank, tag);
            }
        }
    } else {
        omrpc_fatal("no stride (%d) is supported in filename send/recv",
                    stride);
    }
} /* omrpcm_send_recv_filename */

void omrpcm_send_recv_filepointer(omrpc_mpi_handle_t *mp, FILE **dp,
                                  int count, int stride, int rw, int rank, int tag)
{
    int i;

    if(stride <= 1){
        if(rw) { /* recv */
            for(i = 0; i < count; i++){
                omrpcm_recv_filepointer(mp, &(dp[i]), rank, tag);
            }
        } else {
            for(i = 0; i < count; i++){
                omrpcm_send_filepointer(mp, &(dp[i]), rank, tag);
            }
        }
    } else {
        omrpc_fatal("no stride (%d) is supported in filepointer send/recv",
                    stride);
    }
} /* omrpcm_send_recv_filepointer */

static void omrpcm_recv_param_desc(omrpc_mpi_handle_t *mp,
                                  struct ninf_param_desc *dp)
{
    int i, p, tag = 13;
    int ibuf[8+NINF_EXPRESSION_LENGTH*8];

    omrpcm_recv_int(mp, &(ibuf[0]), 3, 0, tag++);
    dp->param_type =ibuf[0];
    dp->param_inout=ibuf[1];
    dp->ndim =ibuf[2];
    for(i = 0; i < dp->ndim; i++){
        p=0;
        omrpcm_recv_int(mp, &(ibuf[0]),8+NINF_EXPRESSION_LENGTH*8, 0, tag++);

        dp->dim[i].size_type=ibuf[p++];
        dp->dim[i].size=ibuf[p++];
        memcpy(dp->dim[i].size_exp.type, &(ibuf[p]), sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;
        memcpy(dp->dim[i].size_exp.val,  &(ibuf[p]), sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;

        dp->dim[i].start_type=ibuf[p++];
        dp->dim[i].start=ibuf[p++];
        memcpy(dp->dim[i].start_exp.type, &(ibuf[p]), sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;
        memcpy(dp->dim[i].start_exp.val,  &(ibuf[p]), sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;

        dp->dim[i].end_type=ibuf[p++];
        dp->dim[i].end=ibuf[p++];
        memcpy(dp->dim[i].end_exp.type, &(ibuf[p]), sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;
        memcpy(dp->dim[i].end_exp.val,  &(ibuf[p]), sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;

        dp->dim[i].step_type=ibuf[p++];
        dp->dim[i].step=ibuf[p++];
        memcpy(dp->dim[i].step_exp.type, &(ibuf[p]), sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;
        memcpy(dp->dim[i].step_exp.val,  &(ibuf[p]), sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH; 
    } 
} /* omrpcm_recv_param_desc */

static void omrpcm_send_param_desc(omrpc_mpi_handle_t *mp,
                                  struct ninf_param_desc *dp)
{
    int i, p, tag = 13;
    int ibuf[8+NINF_EXPRESSION_LENGTH*8];

    ibuf[0]=dp->param_type;
    ibuf[1]=dp->param_inout;
    ibuf[2]=dp->ndim;
    omrpcm_send_int(mp, &(ibuf[0]), 3, 0, tag++);
    for(i = 0; i < dp->ndim; i++){
        p=0;
        ibuf[p++]=dp->dim[i].size_type;
        ibuf[p++]=dp->dim[i].size;
        memcpy(&(ibuf[p]), dp->dim[i].size_exp.type, sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;
        memcpy(&(ibuf[p]), dp->dim[i].size_exp.val,  sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;

        ibuf[p++]=dp->dim[i].start_type;
        ibuf[p++]=dp->dim[i].start;
        memcpy(&(ibuf[p]), dp->dim[i].start_exp.type, sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;
        memcpy(&(ibuf[p]), dp->dim[i].start_exp.val, sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;

        ibuf[p++]=dp->dim[i].end_type;
        ibuf[p++]=dp->dim[i].end;
        memcpy(&(ibuf[p]), dp->dim[i].end_exp.type, sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;
        memcpy(&(ibuf[p]), dp->dim[i].end_exp.val, sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;

        ibuf[p++]=dp->dim[i].step_type;
        ibuf[p++]=dp->dim[i].step;
        memcpy(&(ibuf[p]), dp->dim[i].step_exp.type, sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;
        memcpy(&(ibuf[p]), dp->dim[i].step_exp.val, sizeof(int)*NINF_EXPRESSION_LENGTH);
        p+=NINF_EXPRESSION_LENGTH;

        omrpcm_send_int(mp, &(ibuf[0]), p, 0, tag++);
    }
} /* omrpcm_send_param_desc */

void omrpcm_send_stub_info(omrpc_mpi_handle_t *mp, NINF_STUB_INFO  *sp)
{
    struct ninf_param_desc *dp;
    int  i;
    char c; 

    if(omrpc_debug_flag) omrpc_prf("omrpcm_send_stub_info ...\n");

    c = OMRPC_ACK_OK;

    omrpcm_send_char(mp, &c,                1, 0, mp->tag++);
    omrpcm_send_int(mp, &sp->version_major, 1, 0, mp->tag++);
    omrpcm_send_int(mp, &sp->version_minor, 1, 0, mp->tag++);
    omrpcm_send_int(mp, &sp->info_type,     1, 0, mp->tag++);
    omrpcm_send_str(mp, sp->module_name,       0, mp->tag++); mp->tag++;
    omrpcm_send_str(mp, sp->entry_name,        0, mp->tag++); mp->tag++;
    omrpcm_send_int(mp, &sp->nparam,        1, 0, mp->tag++);
    omrpcm_send_char(mp, &sp->shrink,       1, 0, mp->tag++);
    omrpcm_send_char(mp, &sp->backend,      1, 0, mp->tag++);
    omrpcm_send_short(mp, &sp->index,       1, 0, mp->tag++);

    dp = sp->params;
    for(i = 0; i < sp->nparam; i++){
        omrpcm_send_param_desc(mp,dp++);
    }
} /* omrpcm_send_stub_info */

int omrpcm_recv_stub_info(omrpc_mpi_handle_t *mp, NINF_STUB_INFO  **spp)
{
    char ack;
    NINF_STUB_INFO *sp;
    struct ninf_param_desc *dp;
    int  i;

    if(omrpc_debug_flag) omrpc_prf("omrpcm_recv_stub_info ...\n");

    sp = ninf_new_stub_info();

    omrpcm_recv_char(mp, &ack, 1, 0, mp->tag++);
    if(ack != OMRPC_ACK_OK){
        return OMRPC_ERROR;
    }
    omrpcm_recv_int(mp, &sp->version_major, 1, 0, mp->tag++);
    omrpcm_recv_int(mp, &sp->version_minor, 1, 0, mp->tag++);
    omrpcm_recv_int(mp, &sp->info_type,     1, 0, mp->tag++);
    omrpcm_recv_str(mp, sp->module_name,       0, mp->tag++); mp->tag++;
    omrpcm_recv_str(mp, sp->entry_name,        0, mp->tag++); mp->tag++;
    omrpcm_recv_int(mp, &sp->nparam,        1, 0, mp->tag++);
    omrpcm_recv_char(mp, &sp->shrink,       1, 0, mp->tag++);
    omrpcm_recv_char(mp, &sp->backend,      1, 0, mp->tag++);
    omrpcm_recv_short(mp, &sp->index,       1, 0, mp->tag++);

    dp = ninf_new_param_desc(sp->nparam);
    sp->params = dp;
    for(i = 0; i < sp->nparam; i++){
        omrpcm_recv_param_desc(mp,dp++);
    }
    sp->description = NULL;
    *spp = sp;

    if(omrpc_debug_flag){
        omrpc_prf("get_stub_info:");
        ninf_print_stub_info(stderr,sp);
    }

    return OMRPC_OK;
} /* omrpcm_recv_stub_info */

int omrpcm_req_stub_info(omrpc_mpi_handle_t *mp, char *name)
{
    int    tag, l;
    char   c;

    tag = 0;
    c   = OMRPC_REQ_STUB_INFO;
    l = strlen(name)+1;
    omrpcm_send_char(mp, &c, 1, 0, mp->tag++);
    omrpcm_send_str(mp, name, 0, mp->tag++);

    if(omrpc_debug_flag) omrpc_prf("omrpcm_req_stub_info ...\n");

    return TRUE;
} /* omrpcm_req_stub_info */

/* 
 * send & recv scalar arguments
 */
void ninfm_recv_scalar_args(omrpc_mpi_handle_t *mp, NINF_STUB_INFO *sp,
                           any_t *args, int is_client)
{
    int i, tag;
    struct ninf_param_desc *dp;

    tag = mp->tag;
    /* recv IN scalar variables */
    for(i = 0; i < sp->nparam; i++){
        dp = &sp->params[i];
        if(dp->param_type == DT_FUNC_TYPE)
            omrpc_fatal("call back func not supported");

        if(dp->ndim != 0) continue;
        if(!IS_IN_MODE(dp->param_inout)) break;

        switch(dp->param_type){
        case DT_CHAR:
            omrpcm_recv_char(mp, &args[i].c, 1, 0, tag);
            break;
        case DT_UNSIGNED_CHAR:
            omrpcm_recv_u_char(mp, &args[i].uc, 1, 0, tag);
            break;
        case DT_SHORT:
            omrpcm_recv_short(mp, &args[i].s, 1, 0, tag);
            break;
        case DT_UNSIGNED_SHORT:
            omrpcm_recv_u_short(mp, &args[i].us, 1, 0, tag);
            break;
        case DT_INT:
            omrpcm_recv_int(mp, &args[i].i, 1, 0, tag);
            break;
        case DT_UNSIGNED:
            omrpcm_recv_u_int(mp, &args[i].ui, 1, 0, tag);
            break;
        case DT_LONG:
            omrpcm_recv_long(mp, &args[i].l, 1, 0, tag);
            break;
        case DT_UNSIGNED_LONG:
            omrpcm_recv_u_long(mp, &args[i].ul, 1, 0, tag);
            break;
        case DT_FLOAT:
            omrpcm_recv_float(mp, &args[i].f, 1, 0, tag);
            break;
        case DT_DOUBLE:
            omrpcm_recv_double(mp, &args[i].d, 1, 0, tag);
            break;
        case DT_STRING_TYPE:
            omrpcm_recv_strdup(mp, (char **)(& args[i].p), 0, tag); tag++;
            break;
        case DT_UNSIGNED_LONGLONG:
            omrpcm_recv_long_long(mp, &args[i].ll, 1, 0, tag);
            break;
        case DT_LONGLONG:
            omrpcm_recv_u_long_long(mp, &args[i].ull, 1, 0, tag);
            break;
        case DT_SCOMPLEX:
            omrpcm_recv_float_complex(mp, &args[i].fc, 1, 0, tag);
            break;
        case DT_DCOMPLEX:
            omrpcm_recv_double_complex(mp, &args[i].dc, 1, 0, tag);
            break;
        case DT_LONG_DOUBLE:
            omrpcm_recv_long_double(mp, &args[i].ld, 1, 0, tag);
            break;
        case DT_FILENAME:
            omrpcm_recv_filename(mp,  (char **)(& args[i].p), 0, tag);
            break;
        case DT_FILEPOINTER:
            omrpcm_recv_filepointer(mp, (FILE **)(&args[i].p), 0, tag);
            break;
        default:
            goto err;
            break;
        }
    }
    mp->tag = tag+1;
    return;
err:
    omrpc_fatal("ninf_recv_scalar_args: error");
} /* ninfm_recv_scalar_args */

void ninfm_send_scalar_args(omrpc_mpi_handle_t *mp, NINF_STUB_INFO * sp,
                          any_t *args,int is_client)
{
    int i, tag;
    struct ninf_param_desc *dp;

    tag = mp->tag;
    /* recv IN scalar variables */
    for(i = 0; i < sp->nparam; i++){
        dp = &sp->params[i];
        if(dp->param_type == DT_FUNC_TYPE)
            omrpc_fatal("call back func not supported");

        if(dp->ndim != 0) continue;
        if(!IS_IN_MODE(dp->param_inout)) break;

        switch(dp->param_type){
        case DT_CHAR:
            omrpcm_send_char(mp, &args[i].c, 1, 0, tag);
            break;
        case DT_UNSIGNED_CHAR:
            omrpcm_send_u_char(mp, &args[i].uc, 1, 0, tag);
            break;
        case DT_SHORT:
            omrpcm_send_short(mp, &args[i].s, 1, 0, tag);
            break;
        case DT_UNSIGNED_SHORT:
            omrpcm_send_u_short(mp, &args[i].us, 1, 0, tag);
            break;
        case DT_INT:
            omrpcm_send_int(mp, &args[i].i, 1, 0, tag);
            break;
        case DT_UNSIGNED:
            omrpcm_send_u_int(mp, &args[i].ui, 1, 0, tag);
            break;
        case DT_LONG:
            omrpcm_send_long(mp, &args[i].l, 1, 0, tag);
            break;
        case DT_UNSIGNED_LONG:
            omrpcm_send_u_long(mp, &args[i].ul, 1, 0, tag);
            break;
        case DT_FLOAT:
            omrpcm_send_float(mp, &args[i].f, 1, 0, tag);
            break;
        case DT_DOUBLE:
            omrpcm_send_double(mp, &args[i].d, 1, 0, tag);
            break;
        case DT_STRING_TYPE:
            omrpcm_send_str(mp, args[i].p, 0, tag); tag++;
            break;
        case DT_UNSIGNED_LONGLONG:
            omrpcm_send_long_long(mp, &args[i].ll, 1, 0, tag);
            break;
        case DT_LONGLONG:
            omrpcm_send_u_long_long(mp, &args[i].ull, 1, 0, tag);
            break;
        case DT_SCOMPLEX:
            omrpcm_send_float_complex(mp, &args[i].fc, 1, 0, tag);
            break;
        case DT_DCOMPLEX:
            omrpcm_send_double_complex(mp, &args[i].dc, 1, 0, tag);
            break;
        case DT_LONG_DOUBLE:
            omrpcm_send_long_double(mp, &args[i].ld, 1, 0, tag);
            break;
        case DT_FILENAME:
            omrpcm_send_filename(mp,  (char **)(&args[i].p), 0, tag);
            break;
        case DT_FILEPOINTER:
            omrpcm_send_filepointer(mp,  (FILE **)(&args[i].p), 0, tag);
            break;
        default:
            goto err;
            break;
        }
    }
    mp->tag = tag+1;
    return;
err:
    omrpc_fatal("ninf_send_scalar_args: error");
} /* ninfm_send_scalar_args */

/* 
 * send and recv any values with stride
 */
void ninfm_recv_send_any(omrpc_mpi_handle_t *mp, DATA_TYPE dt,
                         void *p, int count, int stride,int rw,int rank,int tag)
{
    /* recv IN scalar variables */
    switch(dt){
    case DT_CHAR:
        omrpcm_send_recv_char(mp,(char *)p,count,stride,rw,rank,tag);
        break;
    case DT_UNSIGNED_CHAR:
        omrpcm_send_recv_u_char(mp,(unsigned char *)p,count,stride,rw,rank,tag);
        break;
    case DT_SHORT:
        omrpcm_send_recv_short(mp,(short *)p,count,stride,rw,rank,tag);
        break;
    case DT_UNSIGNED_SHORT:
        omrpcm_send_recv_u_short(mp,(unsigned short *)p,count,stride,rw,rank,tag);
        break;
    case DT_INT:
        omrpcm_send_recv_int(mp,(int *)p,count,stride,rw,rank,tag);
        break;
    case DT_UNSIGNED:
        omrpcm_send_recv_u_int(mp,(unsigned int *)p,count,stride,rw,rank,tag);
        break;
    case DT_LONG:
        omrpcm_send_recv_long(mp,(long *)p,count,stride,rw,rank,tag);
        break;
    case DT_UNSIGNED_LONG:
        omrpcm_send_recv_u_long(mp,(unsigned long *)p,count,stride,rw,rank,tag);
        break;
    case DT_FLOAT:
        omrpcm_send_recv_float(mp,(float *)p,count,stride,rw,rank,tag);
        break;
    case DT_DOUBLE:
        omrpcm_send_recv_double(mp,(double *)p,count,stride,rw,rank,tag);
        break;
    case DT_UNSIGNED_LONGLONG:
        omrpcm_send_recv_u_long_long(mp,(unsigned long long *)p,
                                    count,stride,rw,rank,tag);
        break;
    case DT_LONGLONG:
        omrpcm_send_recv_long_long(mp,(long long *)p,
                                     count,stride,rw,rank,tag);
        break;
    case DT_STRING_TYPE:
        omrpcm_send_recv_string(mp,(char **)p,
                               count,stride,rw,rank,tag);
        break;
    case DT_SCOMPLEX:
        omrpcm_send_recv_float_complex(mp,(float _Complex *)p,
                                      count,stride,rw,rank,tag);
        break;
    case DT_DCOMPLEX:   // should not be scalar 
        omrpcm_send_recv_double_complex(mp,(double _Complex *)p,
                                      count,stride,rw,rank,tag);
        break;
    case DT_LONG_DOUBLE:
        omrpcm_send_recv_long_double(mp,(long double *)p,
                                    count, stride, rw,rank,tag);
    case DT_FILENAME:
        omrpcm_send_recv_filename(mp,(char **)p,count,stride,rw,rank,tag);
        break;
    case DT_FILEPOINTER:
        omrpcm_send_recv_filepointer(mp,(FILE **)p,
                                    count,stride,rw,rank,tag);
        break;
    default:
        goto err;
        break;
    }
    return;
err:
    omrpc_fatal("ninfm_recv_send_any: error");
} /* ninfm_recv_send_any */

static void ninfm_trans_array_rec(int dim, omrpc_mpi_handle_t *mp, char *base,
                                  ninf_array_shape_info *ap, int rw, int rank);

void ninfm_recv_array(omrpc_mpi_handle_t *mp, char *base,
                      ninf_array_shape_info *ap, int rank)
{
    if(ap->shrink){
        ninfm_recv_send_any(mp, ap->elem_type,
                           base,ap->total_count,1,OMRPC_IO_R,rank,mp->tag++);
    }
    else {
        ninfm_trans_array_rec(ap->ndim-1,mp,base,ap,OMRPC_IO_R,rank);
    }
} /* ninfm_recv_array */

void ninfm_send_array(omrpc_mpi_handle_t *mp, char *base,
                      ninf_array_shape_info *ap, int rank)
{
    if(ap->shrink){
        ninfm_recv_send_any(mp, ap->elem_type,
                           base,ap->total_count,1,OMRPC_IO_W,rank,mp->tag++);
    } else{
        ninfm_trans_array_rec(ap->ndim-1,mp,base,ap,OMRPC_IO_W,rank);
    }
} /* ninfm_send_array */

static void ninfm_trans_array_rec(int dim, omrpc_mpi_handle_t *mp, char *base,
                                 ninf_array_shape_info *ap,int rw, int rank)
{
  int i,bsize,start,end,step,count;

  start = ap->shape[dim].start;
  end = ap->shape[dim].end;
  step = ap->shape[dim].step;

  if (dim <= 0 ){
      count = (end-start+step-1)/step;
      ninfm_recv_send_any(mp,ap->elem_type,base+start,count,step,rw,rank,mp->tag++);
  }else {
      bsize = ap->shape[dim-1].bsize;
      for(i = start; i < end; i += step){
          ninfm_trans_array_rec(dim-1,mp,base+i*bsize,ap,rw,rank);
      }
  }
} /* ninfm_trans_array_rec */

