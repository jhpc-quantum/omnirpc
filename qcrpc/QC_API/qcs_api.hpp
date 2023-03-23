#ifndef _QCS_API_H_
#define _QCS_API_H_

#include "qcs_types.h"

#ifdef _INC_QCS_QULACS_HPP_
#include"qcs_qulacs.hpp"
#endif

#ifdef _INC_QCS_KET_HPP_
#include"qcs_ket.hpp"
#endif

#define MAX_N_GATES 128 
#define MAX_I_ARGS  8
#define MAX_R_ARGS  8

enum enum_gates{
  _IGate,
  _XGate,
  _YGate,
  _ZGate,
  _HGate,
  _SGate,
  _SdgGate,
  _TGate,
  _TdgGate,
  _SXGate,
  _SXdgGate,
  _SYGate,
  _SYdgGate,
  _CXGate, 
  _CYGate, 
  _CZGate, 
  _SwapGate,
  _RXGate,
  _RYGate,
  _RZGate,
  _U1Gate,
  _U2Gate,
  _U3Gate,
  _CRXGate,
  _CRYGate,
  _CRZGate,
  _CCXGate,
  _NGates, // # of gates 
};

enum enum_simlators{
  _qulacs,
  _braket_riken,
  _rpc_qulacs,
  _rpc_braket_riken,
  _Nsims,
};

typedef struct{
  qint   id;
  int    niarg;
  int    nrarg;
  qint   iarg[MAX_I_ARGS];
  double rarg[MAX_R_ARGS];
}gate_info;

typedef struct{
  // --- common parameters --- 
  int            qcs_id;
  int            nprocs;
  qint           qubits;
  int            ngates; 
  gate_info      gate[MAX_N_GATES]; 
}qcs_info_t;

#ifdef __cplusplus
extern "C" {
#endif

void QC_Init(int *argc, char ***argv, int qubits, int qcs_id);
void QC_Measure();
void QC_Finalize();
void QC_SetNodes(int nprocs);

void IGate(qint target_qubit);
void XGate(qint target_qubit);
void YGate(qint target_qubit);
void ZGate(qint target_qubit);
void HGate(qint target_qubit);
void SGate(qint target_qubit);
void SdgGate(qint target_qubit);
void TGate(qint target_qubit);
void TdgGate(qint target_qubit);
void SXGate(qint target_qubit);
void SXdgGate(qint target_qubit);
void SYGate(qint target_qubit);
void SYdgGate(qint target_qubit);
void CXGate(qint target_qubit, qint control_qubit);
void CYGate(qint target_qubit, qint control_qubit);
void CZGate(qint target_qubit, qint control_qubit);
void SwapGate(qint target_qubit0, qint target_qubit1);
void RXGate(double theta, qint target_qubit);
void RYGate(double theta, qint target_qubit);
void RZGate(double theta, qint target_qubit);
void U1Gate(double theta, qint target_qubit);
void U2Gate(double phi, double lam, qint target_qubit);
void U3Gate(double theta, double phi, double lam, qint target_qubit);
void CRXGate(double theta, qint target_qubit, qint control_qubit);
void CRYGate(double theta, qint target_qubit, qint control_qubit);
void CRZGate(double theta, qint target_qubit, qint control_qubit);
void CCXGate(qint target_qubit, qint control_qubit0, qint control_qubit1);

// --- functions defined in <lib wrapper>.so -- 
void qcs_init_lib(qint nqubits);
void qcs_finalize_lib();
void qcs_measure(qcs_info_t *qcs_info);
void *qcs_update();


void add_IGate(gate_info *gate_info);
void add_XGate(gate_info *gate_info);
void add_YGate(gate_info *gate_info);
void add_ZGate(gate_info *gate_info);
void add_HGate(gate_info *gate_info);
void add_SGate(gate_info *gate_info);
void add_SdgGate(gate_info *gate_info);
void add_TGate(gate_info *gate_info);
void add_TdgGate(gate_info *gate_info);
void add_SXGate(gate_info *gate_info);
void add_SXdgGate(gate_info *gate_info);
void add_SYGate(gate_info *gate_info);
void add_SYdgGate(gate_info *gate_info);
void add_CXGate(gate_info *gate_info);
void add_CYGate(gate_info *gate_info);
void add_CZGate(gate_info *gate_info);
void add_SwapGate(gate_info *gate_info);
void add_RXGate(gate_info *gate_info);
void add_RYGate(gate_info *gate_info);
void add_RZGate(gate_info *gate_info);
void add_U1Gate(gate_info *gate_info);
void add_U2Gate(gate_info *gate_info);
void add_U3Gate(gate_info *gate_info);
void add_CRXGate(gate_info *gate_info);
void add_CRYGate(gate_info *gate_info);
void add_CRZGate(gate_info *gate_info);
void add_CCXGate(gate_info *gate_info);
#ifdef __cplusplus
} // extern "C"
#endif
#endif
