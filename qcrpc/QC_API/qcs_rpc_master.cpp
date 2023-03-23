#include<stdio.h>
#include<stdlib.h>
#include"qcs_api.hpp"
#include"OmniRpc-CPP.h"
#include<iostream>


extern "C" void error(std::string s, std::string fname, int n)
{
  printf("%s %s %d\n", s.c_str(), fname.c_str(), n);
  exit(1);
}

extern "C" void qcs_init_lib(qint n){
//  qi          = (qulacs_info *)malloc(sizeof(qulacs_info));
//  qi->circuit = new QuantumCircuit(nqubits);
//  qi->st      = new QuantumStateCpu(nqubits);
}

extern "C" void qcs_finalize_lib()
{
  //if(qi==NULL)    error("error", __FILE__, __LINE__);
  //if(qi->circuit) delete qi->circuit;
  //if(qi->st)      delete qi->st;
  //delete qi;
}

extern "C" void *qcs_update()
{
  //qi->circuit->update_quantum_state(qi->st);
  //const CPPCTYPE* raw_data_cpp = qi->st->data_cpp();
  //for(int i=0; i<8; i++){
  //  std::cout << raw_data_cpp[i] << std::endl;
  // }
  //return (void *)qi->st->data_c();
  return NULL;
}

extern "C" void qcs_measure(qcs_info_t* qcs_info)
{
  int ngates = qcs_info->ngates;
  int nint   = 0;
  int nreal  = 0;
  OmniRpcRequest rq1;

  
  if(ngates<=0) error("error",__FILE__,__LINE__);

  for(int i=0; i<ngates; i++){
    nint  += (1+1+1+qcs_info->gate[i].niarg);
    nreal += qcs_info->gate[i].nrarg;
  }

  double *rbuf;
  int    *ibuf = (int    *)malloc(sizeof(int)*nint);
  if(ibuf==NULL) error("error",__FILE__,__LINE__);
  if(nreal>0){
    rbuf = (double *)malloc(sizeof(double)*nreal);
    if(rbuf==NULL) error("error",__FILE__,__LINE__);
  }else{
    nreal=1;
    rbuf = (double *)malloc(sizeof(double)*nreal);
    if(rbuf==NULL) error("error",__FILE__,__LINE__);
    rbuf[0]=-1;
  }

  nint  = 0;
  nreal = 0;
   for(int i=0; i<ngates; i++){
    ibuf[nint++] = qcs_info->gate[i].id;
    ibuf[nint++] = qcs_info->gate[i].niarg;
    ibuf[nint++] = qcs_info->gate[i].nrarg;
    for(int j=0; j<qcs_info->gate[i].niarg; j++){
      ibuf[nint++] = qcs_info->gate[i].iarg[j];
    }
    for(int j=0; j<qcs_info->gate[i].nrarg; j++){
      rbuf[nreal++] = qcs_info->gate[i].rarg[j];
    }
  }

  // debug
  printf("ngates=%d nint=%d nreal=%d qcs_id=%d\n",ngates,nint,nreal,_rpc_qulacs);
  // 戻りは？
  if(qcs_info->qcs_id==_rpc_qulacs){
    rq1 = OmniRpcCallAsync("rpc_qc", qcs_info->qubits, ngates, nint, ibuf, nreal, rbuf);
    OmniRpcWait(rq1);
  }
  /* // !!! uncomment if you use braket-lib instead of qulacs-lib !!!
  else if(qcs_info->qcs_id==_rpc_braket_riken){
    rq1 = OmniRpcMpiCallAsync("rpc_mpi_qc", qcs_info->nprocs, qcs_info->qubits, ngates, nint, ibuf, nreal, rbuf);

    int flag = 0; 
    while(!flag){
      flag = OmniRpcProbe(rq1);
    }
  }else{
    // other QC simulators ... 
  }
*/

  free(ibuf); 
  free(rbuf);
}


/*
 These dummy functions are not used in the rpc-master
 */
extern "C" void add_IGate(gate_info *gate_info){}
extern "C" void add_XGate(gate_info *gate_info){}
extern "C" void add_YGate(gate_info *gate_info){}
extern "C" void add_ZGate(gate_info *gate_info){}
extern "C" void add_HGate(gate_info *gate_info){}
extern "C" void add_SGate(gate_info *gate_info){}
extern "C" void add_SdgGate(gate_info *gate_info){}
extern "C" void add_TGate(gate_info *gate_info){}
extern "C" void add_TdgGate(gate_info *gate_info){}
extern "C" void add_SXGate(gate_info *gate_info){}
extern "C" void add_SXdgGate(gate_info *gate_info){}
extern "C" void add_SYGate(gate_info *gate_info){}
extern "C" void add_SYdgGate(gate_info *gate_info){}
extern "C" void add_CXGate(gate_info *gate_info){}
extern "C" void add_CYGate(gate_info *gate_info){}
extern "C" void add_CZGate(gate_info *gate_info){}
extern "C" void add_SwapGate(gate_info *gate_info){}
extern "C" void add_RXGate(gate_info *gate_info){}
extern "C" void add_RYGate(gate_info *gate_info){}
extern "C" void add_RZGate(gate_info *gate_info){}
extern "C" void add_U1Gate(gate_info *gate_info){}
extern "C" void add_U2Gate(gate_info *gate_info){}
extern "C" void add_U3Gate(gate_info *gate_info){}
extern "C" void add_CRXGate(gate_info *gate_info){}
extern "C" void add_CRYGate(gate_info *gate_info){}
extern "C" void add_CRZGate(gate_info *gate_info){}
extern "C" void add_CCXGate(gate_info *gate_info){}
