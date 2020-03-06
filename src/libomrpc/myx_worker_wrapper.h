#define MYX

#ifdef MYX
#define MPI_Comm_rank(comm, rank)\
       PMPI_Comm_rank(comm, rank)

#define MPI_Bcast(buf, count, type, root, comm)\
       PMPI_Bcast(buf, count, type, root, comm)

#define MPI_Barrier(comm)\
       PMPI_Barrier(comm)

#endif // #ifdef MYX
