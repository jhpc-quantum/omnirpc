#pragma once

#include "qcs_types.h"

#ifdef _INC_QCS_QULACS_HPP_
#include"qcs_qulacs.hpp"
#endif

#ifdef _INC_QCS_KET_HPP_
#include"qcs_ket.hpp"
#endif

__BEGIN_DECLS


void QC_Init(int *argc, char ***argv, int qubits, int qcs_id);
void QC_Measure(void);
void QC_Finalize(void);
void QC_SetNodes(int nprocs);

void QC_InitRemote(int *argc, char **argv[]);
void QC_MeasureRemote(void);
void QC_SaveContext(const char *file);
void QC_LoadContext(const char *file);

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
void qcs_finalize_lib(void);
void qcs_measure(qcs_info_t *qcs_info);
void *qcs_update(void);

void add_IGate(gate_info *ginfo);
void add_XGate(gate_info *ginfo);
void add_YGate(gate_info *ginfo);
void add_ZGate(gate_info *ginfo);
void add_HGate(gate_info *ginfo);
void add_SGate(gate_info *ginfo);
void add_SdgGate(gate_info *ginfo);
void add_TGate(gate_info *ginfo);
void add_TdgGate(gate_info *ginfo);
void add_SXGate(gate_info *ginfo);
void add_SXdgGate(gate_info *ginfo);
void add_SYGate(gate_info *ginfo);
void add_SYdgGate(gate_info *ginfo);
void add_CXGate(gate_info *ginfo);
void add_CYGate(gate_info *ginfo);
void add_CZGate(gate_info *ginfo);
void add_SwapGate(gate_info *ginfo);
void add_RXGate(gate_info *ginfo);
void add_RYGate(gate_info *ginfo);
void add_RZGate(gate_info *ginfo);
void add_U1Gate(gate_info *ginfo);
void add_U2Gate(gate_info *ginfo);
void add_U3Gate(gate_info *ginfo);
void add_CRXGate(gate_info *ginfo);
void add_CRYGate(gate_info *ginfo);
void add_CRZGate(gate_info *ginfo);
void add_CCXGate(gate_info *ginfo);

__END_DECLS

