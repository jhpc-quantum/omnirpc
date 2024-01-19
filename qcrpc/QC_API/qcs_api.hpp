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

qcs_info_t *QC_GetContext(void);
void QC_InitRemote(int *argc, char **argv[]);
void QC_MeasureRemote(bool submit_job);
void QC_SaveContext(const char *file);
void QC_LoadContext(const char *file);
void QC_MeasureRemoteQASMFile(const char *dir, const char *file,
                              int *qbit_ptns, double *n_ptns, int buflen, 
                              int *n_shots);
void QC_MeasureRemoteQASMStringREST(const char *url, const char *token,
                                    const char *qasm, int qc_type,
                                    const char *rem,
                                    int shots, int poll_ms, int poll_max,
                                    int transpiler,
                                    char *out, int maxout, int *olen);
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

__END_DECLS

