
/*
 library for exec by mpi_comm_spawn
 M.Tsuji 2011.09.08
*/
#include<mpi.h>
#include<stdio.h>
#include"../../include/omrpc_mpi_lib.h"
#include"myx_master_wrapper.h"

void omrpc_recv_nodelist(int nprocs, MPI_Comm comm, omrpc_mpi_node_t *nodes)
{
  int           count=3;
  int           array_of_blocklengths[3];
  MPI_Aint      array_of_displacements[3];
  MPI_Datatype  array_of_types[3];
  MPI_Datatype  newtype;
  MPI_Status    status;

  array_of_blocklengths[0]=1;
  array_of_blocklengths[1]=4;
  array_of_blocklengths[2]=MAXPATHLEN;
  array_of_displacements[0]=0;
  array_of_displacements[1]=sizeof(int);
  array_of_displacements[2]=array_of_displacements[1]+4*sizeof(int);
  array_of_types[0]=MPI_INT;
  array_of_types[1]=MPI_INT;
  array_of_types[2]=MPI_CHAR;

  MPI_Type_create_struct(count,array_of_blocklengths,array_of_displacements,array_of_types,&newtype);
  MPI_Type_commit(&newtype);

  MPI_Recv(nodes,nprocs,newtype,0,0,comm,&status);
} /* omrpc_recv_nodelist */

void omrpc_send_nodelist(int nprocs, MPI_Comm comm, omrpc_mpi_node_t *nodes)
{
  int           count=3;
  int           array_of_blocklengths[3];
  MPI_Aint      array_of_displacements[3];
  MPI_Datatype  array_of_types[3];
  MPI_Datatype  newtype;
  MPI_Status    status;

  array_of_blocklengths[0]=1;
  array_of_blocklengths[1]=4;
  array_of_blocklengths[2]=MAXPATHLEN;
  array_of_displacements[0]=0;
  array_of_displacements[1]=sizeof(int);
  array_of_displacements[2]=array_of_displacements[1]+4*sizeof(int);
  array_of_types[0]=MPI_INT;
  array_of_types[1]=MPI_INT;
  array_of_types[2]=MPI_CHAR;

  MPI_Type_create_struct(count,array_of_blocklengths,array_of_displacements,array_of_types,&newtype);

  MPI_Type_commit(&newtype);

  MPI_Send(nodes,nprocs,newtype,0,0,comm);

} /* omrpc_send_nodelist */

