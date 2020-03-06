/* prototype */
void omrpc_stub_INIT(int argc, char * argv[]);
int omrpc_stub_REQ(void);
void omrpc_stub_SET_ARG(void  *cp,int arg_i);
void omrpc_stub_BEGIN(void);
void omrpc_stub_END(void);
void omrpc_stub_EXIT(void);
void omrpc_stub_EXIT_MPI();
#ifdef USE_FT
char OmniRpcMpiAskHandleAlive(short id);
#endif
