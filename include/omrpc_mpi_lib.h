/*
 library for exec by mpi_comm_spawn
 M.Tsuji 2011.09.08
*/
#ifndef OMRPC_MPI_LIB
#define OMRPC_MPI_LIB

#include<mpi.h>

#define MAXPATHLEN 256

typedef struct omrpc_mpi_node{
  int rank;
  int ip[4];
  char name[MAXPATHLEN];
} omrpc_mpi_node_t;

void omrpc_recv_nodelist(int nprocs, MPI_Comm comm, omrpc_mpi_node_t *nodes);
void omrpc_send_nodelist(int nprocs, MPI_Comm comm, omrpc_mpi_node_t *nodes);

#endif // OMRPC_MPI_LIB
