#define MYX

#ifdef MYX

#define MPI_Init(argc, argv)\
       PMPI_Init(argc, argv)

#define MPI_Finalize()\
       PMPI_Finalize()

#define MPI_Comm_spawn(command, argv, maxprocs, info, root, comm, intercomm, array_of_errcodes)\
       omrpc_prf("nprocs=%d+1\n",maxprocs);fflush(stdout);PMPI_Comm_spawn(command, argv, maxprocs+1, info, root, comm, intercomm, array_of_errcodes)

#define MPI_Barrier(comm)\
       PMPI_Barrier(comm)

#define MPI_Send(buf, count, datatype, dest, tag, comm)\
       PMPI_Send(buf, count, datatype, dest, tag, comm)

#define MPI_Recv(buf, count, datatype, dest, tag, comm, stat)\
       PMPI_Recv(buf, count, datatype, dest, tag, comm, stat)

#define MPI_Comm_rank(world, rank)\
       PMPI_Comm_rank(world, rank)

#define MPI_Comm_size(world, size)\
       PMPI_Comm_size(world, size)

#define MPI_Wtime()\
       PMPI_Wtime()

#define MPI_Iprobe(src, tag, comm, flag, stat)\
       PMPI_Iprobe(src, tag, comm, flag, stat)

#define MPI_Open_port(info, port)\
       PMPI_Open_port(info, port)

#define MPI_Comm_accept(port_name, info, root, comm, newcomm)\
       PMPI_Comm_accept(port_name, info, root, comm, newcomm)

#define MPI_Comm_connect(port_name, info, root, comm, newcomm)\
       PMPI_Comm_connect(port_name, info, root, comm, newcomm)

#define MPI_Type_struct(count, array_of_blocklengths, array_of_displacements, array_of_types, newtype)\
       PMPI_Type_struct(count, array_of_blocklengths, array_of_displacements, array_of_types, newtype)

#define MPI_Type_commit(type)\
       PMPI_Type_commit(type)

#define MPI_Bcast(buf, count, type, root, comm)\
       PMPI_Bcast(buf, count, type, root, comm)

#define MPI_Test(req, flag, stat)\
       PMPI_Test(req, flag, stat)

#define MPI_Comm_get_parent(comm)\
       PMPI_Comm_get_parent(comm)

#define MPI_Comm_free(comm)\
       PMPI_Comm_free(comm)

#define MPI_Comm_split(comm, color, key, newcomm)\
       PMPI_Comm_split(comm, color, key, newcomm)  
/*
#define MPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm)\
       PMPI_Gather(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, root, comm)
*/
#endif // #ifdef MYX
