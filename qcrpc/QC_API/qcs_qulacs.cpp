#include "omni_platform.h"
#include "qcs_types.h"
#if 0
#include <complex.h>
#endif
#include"qcs_qulacs.hpp"

#define _INC_QCS_QULACS_HPP_
#include"qcs_api.hpp"
#undef _INC_QCS_QULACS_HPP_
#include"qcs_gate_funcs.hpp"

static qulacs_info_t qul_info[MAX_INFO];
static ssize_t n_infos = 0;
static pthread_key_t idx_key;
static pthread_once_t once = PTHREAD_ONCE_INIT;

#define _DEBUG_OUT_ 

#ifdef _DEBUG_OUT_
#define _DEBUG_FUNC_OUT()  printf("%s :",__func__); \
  for(int i=0; i<ginfo->niarg; i++){ \
    fprintf(stderr, "%d ",ginfo->iarg[i]);   \
  } \
  for(int i=0; i<ginfo->nrarg; i++){	\
    fprintf(stderr, "%f ",ginfo->rarg[i]);    \
  } \
  printf("\n")
#else
#define _DEBUG_FUNC_OUT() ""
#endif

void error(std::string s, std::string fname, int n)
{
  fprintf(stderr, "%s %s %d\n", s.c_str(), fname.c_str(), n);
  exit(1);
}

static void once_proc(void) {
  int st = pthread_key_create(&idx_key, NULL);
  if (unlikely(st != 0)) {
    fprintf(stderr, "Error: can't crete info index key.\n");
    exit(1);
  }
}

static inline qulacs_info_t *qcs_get_qul_info(void) {
  qulacs_info_t *ret = (qulacs_info_t *)pthread_getspecific(idx_key);
  if (likely(ret != NULL &&
             ret >= &qul_info[0] && ret < &qul_info[MAX_INFO - 1])) {
    return ret;
  } else {
    ssize_t idx;
    if (likely((idx = __atomic_fetch_add(&n_infos, 1, __ATOMIC_ACQ_REL))
               >= 0 &&
               idx < MAX_INFO && n_infos > 0)) {
      int st = pthread_setspecific(idx_key, &qul_info[idx]);
      if (likely(st == 0)) {
        ret = &qul_info[idx];
        (void)memset(ret, 0, sizeof(*ret));
        return ret;
      } else {
        fprintf(stderr, "Error: can't set info index.\n");
        exit(1);
      }
    } else {
      if (idx >= MAX_INFO) {
        fprintf(stderr, "Error: too many info, < %d.\n", MAX_INFO);
      }
      exit(1);
    }
  }
}

void qcs_init_lib(qint nqubits) {
  qulacs_info_t *qi = NULL;
  (void)pthread_once(&once, once_proc);

  if (unlikely((qi = qcs_get_qul_info()) != NULL)) {
    qi->nqubits = nqubits;
    qi->circuit = new QuantumCircuit(nqubits);
    qi->st = new QuantumStateCpu(nqubits);
  } else {
    fprintf(stderr, "Error: qulacs object holder allocation failed.\n");
    exit(1);
  }
}

void qcs_finalize_lib(void)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  if (qi == NULL) {
    error("error", __FILE__, __LINE__);
  }

  if (qi->circuit) {
#if 0
    delete qi->circuit;
#endif
  }
  if (qi->st) {
#if 0
    delete qi->st;
#endif
  }
  (void)memset(qi, 0, sizeof(*qi));

  (void)pthread_setspecific(idx_key, NULL);
}

void *qcs_update()
{
  qulacs_info_t *qi = qcs_get_qul_info();

  qi->circuit->update_quantum_state(qi->st);
  const CPPCTYPE* raw_data_cpp = qi->st->data_cpp();
  for(int i=0; i<8; i++){
    std::cout << raw_data_cpp[i] << std::endl;
  }
  return (void *)qi->st->data_c();
}

void qcs_measure(qcs_info_t *qcs_info)
{
  qint qubits = qcs_info->qubits;
  int n = qcs_info->ngates;

  qcs_init_lib(qubits);

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

  qcs_finalize_lib();
}



void add_IGate(gate_info *ginfo)
{
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  // I gateはなにもしない
}

void add_XGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();

  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_X_gate(ginfo->iarg[0]);
}

void add_YGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();

  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_Y_gate(ginfo->iarg[0]);
}

void add_ZGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();

  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_Z_gate(ginfo->iarg[0]);
}

void add_HGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_H_gate(ginfo->iarg[0]);
}

void add_SGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_S_gate(ginfo->iarg[0]);
}

void add_SdgGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_Sdag_gate(ginfo->iarg[0]);
}

void add_TGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
    
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_T_gate(ginfo->iarg[0]);
}

void add_TdgGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();

  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_Tdag_gate(ginfo->iarg[0]);
}

void add_SXGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();

  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_sqrtX_gate(ginfo->iarg[0]);
}

void add_SXdgGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();

  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_sqrtXdag_gate(ginfo->iarg[0]);
}

void add_SYGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_sqrtY_gate(ginfo->iarg[0]);
}

void add_SYdgGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
  qi->circuit->add_sqrtYdag_gate(ginfo->iarg[0]);

}

void add_CXGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  // Note: the order of arguments should be changed!
  // extern "C" void CXGate(qint target_qubit, qint control_qubit)
  // --->
  // virtual void add_CNOT_gate(UINT control_index, UINT target_index);
  qi->circuit->add_CNOT_gate(ginfo->iarg[1], ginfo->iarg[0]);

}

void add_CYGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  error("error: The CYGate has not supported in qulacs ",__FILE__,__LINE__);
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
}

void add_CZGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  // Note: the order of arguments should be changed!
  // extern "C" void CZGate(qint target_qubit, qint control_qubit)
  // --->
  // virtual void add_CZ_gate(UINT control_index, UINT target_index);
  qi->circuit->add_CZ_gate(ginfo->iarg[1], ginfo->iarg[0]);
}

void add_SwapGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_SWAP_gate(ginfo->iarg[0], ginfo->iarg[1]);
}

void add_RXGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_RX_gate(ginfo->iarg[0], ginfo->rarg[0]);
}

void add_RYGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_RY_gate(ginfo->iarg[0], ginfo->rarg[0]);
}

void add_RZGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_RZ_gate(ginfo->iarg[0], ginfo->rarg[0]);
}

void add_U1Gate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_U1_gate(ginfo->iarg[0], ginfo->rarg[0]);
}

void add_U2Gate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_U2_gate(ginfo->iarg[0], ginfo->rarg[0], ginfo->rarg[1]);
}

void add_U3Gate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
    
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }    
  qi->circuit->add_U3_gate(ginfo->iarg[0], ginfo->rarg[0], ginfo->rarg[1], ginfo->rarg[2]);
}

void add_CRXGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }   

  //qcs_info->gate[n].iarg[0] = target_qubit;
  //qcs_info->gate[n].iarg[1] = control_qubit;
  std::vector<UINT> target_index_list(ginfo->iarg[0], ginfo->iarg[1]);
  std::vector<UINT> pauli_id_list(1); // (I,X,Y,Z)<->(0,1,2,3)
  double            theta(ginfo->rarg[0]);
  qi->circuit->add_multi_Pauli_rotation_gate(target_index_list, pauli_id_list, theta);
}

void add_CRYGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
    
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }   

  //qcs_info->gate[n].iarg[0] = target_qubit;
  //qcs_info->gate[n].iarg[1] = control_qubit;
  std::vector<UINT> target_index_list(ginfo->iarg[0], ginfo->iarg[1]);
  std::vector<UINT> pauli_id_list(2); // (I,X,Y,Z)<->(0,1,2,3)
  double            theta(ginfo->rarg[0]);
  qi->circuit->add_multi_Pauli_rotation_gate(target_index_list, pauli_id_list, theta);
}

void add_CRZGate(gate_info *ginfo)
{
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }   

  //qcs_info->gate[n].iarg[0] = target_qubit;
  //qcs_info->gate[n].iarg[1] = control_qubit;
  std::vector<UINT> target_index_list(ginfo->iarg[0], ginfo->iarg[1]);
  std::vector<UINT> pauli_id_list(3); // (I,X,Y,Z)<->(0,1,2,3)
  double            theta(ginfo->rarg[0]);
  qi->circuit->add_multi_Pauli_rotation_gate(target_index_list, pauli_id_list, theta);
}

void add_CCXGate(gate_info *ginfo){
  qulacs_info_t *qi = qcs_get_qul_info();
  
  _DEBUG_FUNC_OUT();
  error("error: The CCXGate has not supported in qulacs ",__FILE__,__LINE__);
  if(ginfo==NULL){
    error("error",__FILE__,__LINE__);
  }
}

