/* 
 * prototype for API
 * $Id: OmniRpc.h,v 1.1.1.1 2004-11-03 21:01:36 yoshihiro Exp $
 */
#include "omni_platform.h"

#define OMRPC_OK 0
#define OMRPC_ERROR (-1)

#define INSPECT_PACKET 0

/* low-level and no-MT API */
typedef void *OmniRpcExecHandle;

void OmniRpcExecInit(int *argc, char **argv[]);
void OmniRpcExecFinalize(void);
int OmniRpcExecLocal(char *prog_name, char *func_name, ...);
int OmniRpcExecRemote(char *host, char *prog_name, char *func_name, ...);
int OmniRpcExecRemoteByUser(char *host, char *user_name, char *prog_name, char *func_name, ...);

OmniRpcExecHandle OmniRpcExecOnHost(char *host_name,char *prog_name);
OmniRpcExecHandle OmniRpcExecOnHostByUser(char *host_name,char *user_name,char *prog_name);
int OmniRpcExecCall(OmniRpcExecHandle handle, char *func_name, ...);
void OmniRpcExecTerminate(OmniRpcExecHandle handle);

int OmniRpcModuleInit(char *module_name,...);
void OmniRpcModuleInitV(char *module_name,va_list ap);

/* RPC API */
typedef void *OmniRpcRequest;

void OmniRpcInit(int *argc, char **argv[]);
void OmniRpcFinalize(void);

int OmniRpcCall(char *entry_name,...);
int OmniRpcCallV(char *entry_name,va_list ap);
OmniRpcRequest OmniRpcCallAsync(char *entry_name,...);
void *OmniRpcCallAsyncV(char *entry_name,va_list ap);
int OmniRpcWait(OmniRpcRequest req);
int OmniRpcProbe(OmniRpcRequest req);
int OmniRpcWaitAll(int n, OmniRpcRequest reqs[]);
int OmniRpcWaitAny(int n, OmniRpcRequest reqs[]);

typedef void *OmniRpcHandle;

OmniRpcHandle OmniRpcCreateHandle(char *host_name, char *module_name);
int OmniRpcCallByHandle(OmniRpcHandle handle,char *entry_name,...);
OmniRpcRequest OmniRpcCallAsyncByHandle(OmniRpcHandle handle,char *entry_name,...);
int OmniRpcCallByHandleV(OmniRpcHandle handle, char *entry_name,va_list ap);
OmniRpcRequest OmniRpcCallAsyncByHandleV(OmniRpcHandle handle, 
					 char *entry_name,va_list ap);
void OmniRpcDestroyHandle(OmniRpcHandle handle);
void OmniRpcDestroyRequest(OmniRpcRequest req0);

#ifdef USE_MPI
/*********************************************************
 * APIs for RPC with parallel-remote-programs            *
 *********************************************************/
void           OmniRpcMpiInit(int *argc, char **argv[]);
void           OmniRpcMpiFinalize(void);
OmniRpcHandle  OmniRpcMpiCreateHandle(int nprocs, char *module_name);
void          *OmniRpcMpiCallAsyncByHandle(void *handle,int nprocs,char *entry_name,...);
void          *OmniRpcMpiCallAsyncByHandleV(void *handle,int nprocs,char *entry_name,va_list ap);
void          *OmniRpcMpiCallAsync(char *entry_name,int nprocs,...);
void          *OmniRpcCallAsyncMPI(char *entry_name,int nprocs,...);
MPI_Comm       OmniRpcMpiGetComm(void *handle);
int            OmniRpcMpi_Send(void *buf, int count, MPI_Datatype datatype, 
                               int dest, int tag,MPI_Comm comm);
int            OmniRpcMpi_Recv(void *buf, int count, MPI_Datatype datatype, 
			       int source, int tag,MPI_Comm comm, MPI_Status *status);
int            OmniRpcMpi_Bcast(void *buf, int count, MPI_Datatype datatype,
                               int root, MPI_Comm comm);
int            OmniRpcMpi_Test(MPI_Request *request, int *flag, MPI_Status *status);
#ifdef USE_FT
int            OmniRpcMpiGetErrorHandleId(int ierr);
int            OmniRpcMpiCheckHandle(void *handle);
int            OmniRpcMpiStopHeartBeat(void *handle);
int            OmniRpcMpiAskHandleAlive(int id);
#endif
short          OmniRpcGetRPID(void *handle);
#ifdef USE_FJMPI
int OmniRpcMpi_FJMPISend(void *buf, int count, MPI_Datatype datatype, 
			 int dest, int tag,MPI_Comm comm,char *port);
int OmniRpcMpi_FJMPIRecv(void *buf, int count, MPI_Datatype datatype, 
			 int source, int tag,MPI_Comm comm, MPI_Status *status, char *port);
char *OmniRpcMpiGetPort(void *handle);
#endif

/*********************************************************
 * OmniRpcCallAsyncMPI & OmniRpcCallAsyncVMPI are legacy *
 * functions. Generally, you don't have to use them      *
 *********************************************************/
void *OmniRpcCallAsyncMPI(char *entry_name,int nprocs,...);
void *OmniRpcCallAsyncVMPI(char *entry_name,int nprocs,va_list ap);
#endif /* USE_MPI */
