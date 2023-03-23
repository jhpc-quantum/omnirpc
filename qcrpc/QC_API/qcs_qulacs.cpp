#include<stdio.h>
#include<stdlib.h>
#if 0
#include <complex.h>
#endif
#include"qcs_qulacs.hpp"

#define _INC_QCS_QULACS_HPP_
#include"qcs_api.hpp"
#undef _INC_QCS_QULACS_HPP_
#include"qcs_gate_funcs.hpp"

#define _DEBUG_OUT_ 

#ifdef _DEBUG_OUT_
#define _DEBUG_FUNC_OUT()  printf("%s :",__func__); \
  for(int i=0; i<gate_info->niarg; i++){ \
    printf("%d ",gate_info->iarg[i]); \
  } \
  for(int i=0; i<gate_info->nrarg; i++){	\
    printf("%f ",gate_info->rarg[i]); \
  } \
  printf("\n")
#else
#define _DEBUG_FUNC_OUT() ""
#endif

qulacs_info *qi; 

extern "C" void error(std::string s, std::string fname, int n)
{
  printf("%s %s %d\n", s.c_str(), fname.c_str(), n);
  exit(1);
}

extern "C" void qcs_init_lib(qint nqubits){
  qi          = (qulacs_info *)malloc(sizeof(qulacs_info));
  qi->circuit = new QuantumCircuit(nqubits);
  qi->st      = new QuantumStateCpu(nqubits);
}

extern "C" void qcs_finalize_lib()
{
  if(qi==NULL)    error("error", __FILE__, __LINE__);
  if(qi->circuit) delete qi->circuit;
  if(qi->st)      delete qi->st;
  delete qi;
}

extern "C" void *qcs_update()
{
  qi->circuit->update_quantum_state(qi->st);
  const CPPCTYPE* raw_data_cpp = qi->st->data_cpp();
  for(int i=0; i<8; i++){
    std::cout << raw_data_cpp[i] << std::endl;
  }
  return (void *)qi->st->data_c();
}

extern "C" void qcs_measure(qcs_info_t *qcs_info)
{
  int n = qcs_info->ngates;
  // 一気に gate を add or apply する
  for(int i=0; i<n; i++){
    void (*f)(gate_info*) = gate_func[qcs_info->gate[i].id];
    f(&(qcs_info->gate[i]));
  }

  // addの後に update するもの 
    CTYPE *state = (CTYPE *)qcs_update();
    for(int i=0; i<8; i++){
#if 0
        printf("%e %e\n",creal(state[i]),cimag(state[i]));
#else
        printf("%e %e\n",_creal(state[i]),_cimag(state[i]));
#endif
    }
}


extern "C" void add_IGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  // I gateはなにもしない
}

extern "C" void add_XGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_X_gate(gate_info->iarg[0]);
}

extern "C" void add_YGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_Y_gate(gate_info->iarg[0]);
}

extern "C" void add_ZGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_Z_gate(gate_info->iarg[0]);
}

extern "C" void add_HGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_H_gate(gate_info->iarg[0]);
}

extern "C" void add_SGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_S_gate(gate_info->iarg[0]);
}

extern "C" void add_SdgGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_Sdag_gate(gate_info->iarg[0]);
}

extern "C" void add_TGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_T_gate(gate_info->iarg[0]);
}

extern "C" void add_TdgGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_Tdag_gate(gate_info->iarg[0]);
}

extern "C" void add_SXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_sqrtX_gate(gate_info->iarg[0]);
}

extern "C" void add_SXdgGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_sqrtXdag_gate(gate_info->iarg[0]);
}

extern "C" void add_SYGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_sqrtY_gate(gate_info->iarg[0]);
}

extern "C" void add_SYdgGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_sqrtYdag_gate(gate_info->iarg[0]);

}

extern "C" void add_CXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  // Note: the order of arguments should be changed!
  // extern "C" void CXGate(qint target_qubit, qint control_qubit)
  // --->
  // virtual void add_CNOT_gate(UINT control_index, UINT target_index);
  qi->circuit->add_CNOT_gate(gate_info->iarg[1], gate_info->iarg[0]);

}

extern "C" void add_CYGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  error("error: The CYGate has not supported in qulacs ",__FILE__,__LINE__);
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
}

extern "C" void add_CZGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  // Note: the order of arguments should be changed!
  // extern "C" void CZGate(qint target_qubit, qint control_qubit)
  // --->
  // virtual void add_CZ_gate(UINT control_index, UINT target_index);
  qi->circuit->add_CZ_gate(gate_info->iarg[1], gate_info->iarg[0]);
}

extern "C" void add_SwapGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_SWAP_gate(gate_info->iarg[0], gate_info->iarg[1]);
}

extern "C" void add_RXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_RX_gate(gate_info->iarg[0], gate_info->rarg[0]);
}

extern "C" void add_RYGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_RY_gate(gate_info->iarg[0], gate_info->rarg[0]);
}

extern "C" void add_RZGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_RZ_gate(gate_info->iarg[0], gate_info->rarg[0]);
}

extern "C" void add_U1Gate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_U1_gate(gate_info->iarg[0], gate_info->rarg[0]);
}

extern "C" void add_U2Gate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_U2_gate(gate_info->iarg[0], gate_info->rarg[0], gate_info->rarg[1]);
}

extern "C" void add_U3Gate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_U3_gate(gate_info->iarg[0], gate_info->rarg[0], gate_info->rarg[1], gate_info->rarg[2]);
}

extern "C" void add_CRXGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }   

  //qcs_info->gate[n].iarg[0] = target_qubit;
  //qcs_info->gate[n].iarg[1] = control_qubit;
   std::vector<UINT> target_index_list(gate_info->iarg[0], gate_info->iarg[1]);
   std::vector<UINT> pauli_id_list(1); // (I,X,Y,Z)<->(0,1,2,3)
   double            theta(gate_info->rarg[0]);
   qi->circuit->add_multi_Pauli_rotation_gate(target_index_list, pauli_id_list, theta);
}

extern "C" void add_CRYGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }   

  //qcs_info->gate[n].iarg[0] = target_qubit;
  //qcs_info->gate[n].iarg[1] = control_qubit;
   std::vector<UINT> target_index_list(gate_info->iarg[0], gate_info->iarg[1]);
   std::vector<UINT> pauli_id_list(2); // (I,X,Y,Z)<->(0,1,2,3)
   double            theta(gate_info->rarg[0]);
   qi->circuit->add_multi_Pauli_rotation_gate(target_index_list, pauli_id_list, theta);
}

extern "C" void add_CRZGate(gate_info *gate_info)
{
  _DEBUG_FUNC_OUT();
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }   

  //qcs_info->gate[n].iarg[0] = target_qubit;
  //qcs_info->gate[n].iarg[1] = control_qubit;
   std::vector<UINT> target_index_list(gate_info->iarg[0], gate_info->iarg[1]);
   std::vector<UINT> pauli_id_list(3); // (I,X,Y,Z)<->(0,1,2,3)
   double            theta(gate_info->rarg[0]);
   qi->circuit->add_multi_Pauli_rotation_gate(target_index_list, pauli_id_list, theta);
}

extern "C" void add_CCXGate(gate_info *gate_info){
  _DEBUG_FUNC_OUT();
  error("error: The CCXGate has not supported in qulacs ",__FILE__,__LINE__);
  if(gate_info==NULL){
    error("error",__FILE__,__LINE__);
  }
}

