#include<stdio.h>
#include<stdlib.h>
#include"qcs_api.hpp"
#include"qcs_qulacs.hpp"

#define MAX_THREADS 48
qcs_info_t qcs_info[MAX_THREADS];
int thread_id;

#ifndef _OPENMP
int omp_get_thread_num(){return 0;}
int omp_get_num_threads(){return 1;}
#endif

extern "C" void QC_Init(int *argc, char ***argv, qint qubits, int qcs_id)
{
  if(qcs_id>=_Nsims || qcs_id<0){
    printf("Error: simultion id number is invalid\n");
  }

#pragma omp parallel
{
  thread_id = omp_get_thread_num();
}

  // 必要な部分を初期化 （ここでは仮にqulacsを用いる初期化）
  qcs_info[thread_id].qcs_id  = qcs_id;
  qcs_info[thread_id].qubits  = qubits;
  qcs_info[thread_id].ngates  = 0; 
  qcs_info[thread_id].nprocs  = 1;
  qcs_init_lib(qubits); // 特定のライブラリを初期化
}

extern "C" void QC_n_gate_check(int n)
{
  if(n>=MAX_N_GATES){
    fprintf(stderr,"max gate number %d exceed\n",MAX_N_GATES);
    exit(1);
  }
}

extern "C" void QC_SetNodes(int nprocs)
{
  qcs_info[thread_id].nprocs = nprocs;
}

extern "C" void QC_Finalize()
{
  qcs_finalize_lib();
}

extern "C" void QC_Measure()
{
  qcs_measure(&(qcs_info[thread_id]));
}

extern "C" void IGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _IGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void XGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _XGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void ZGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _ZGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void HGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _HGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void SGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _SGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void SdgGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _SdgGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void TGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _TGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void TdgGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _TdgGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void SXGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _SXGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void SXdgGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _SXdgGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void SYGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _SYGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void SYdgGate(qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _SYdgGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void CXGate(qint target_qubit, qint control_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _CXGate;
  qcs_info[thread_id].gate[n].niarg   = 2; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].iarg[1] = control_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void CYGate(qint target_qubit, qint control_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _CYGate;
  qcs_info[thread_id].gate[n].niarg   = 2; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].iarg[1] = control_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void CZGate(qint target_qubit, qint control_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _CZGate;
  qcs_info[thread_id].gate[n].niarg   = 2; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].iarg[1] = control_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void SwapGate(qint target_qubit0, qint target_qubit1)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _SwapGate;
  qcs_info[thread_id].gate[n].niarg   = 2; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit0;
  qcs_info[thread_id].gate[n].iarg[1] = target_qubit1;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}

extern "C" void RXGate(double theta, qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _RXGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 1; 
  qcs_info[thread_id].gate[n].rarg[0] = theta;
  qcs_info[thread_id].ngates++;
}

extern "C" void RYGate(double theta, qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _RYGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 1; 
  qcs_info[thread_id].gate[n].rarg[0] = theta;
  qcs_info[thread_id].ngates++;
}

extern "C" void RZGate(double theta, qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _RZGate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 1; 
  qcs_info[thread_id].gate[n].rarg[0] = theta;
  qcs_info[thread_id].ngates++;
}

extern "C" void U1Gate(double theta, qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _U1Gate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 1; 
  qcs_info[thread_id].gate[n].rarg[0] = theta;
  qcs_info[thread_id].ngates++;
}

extern "C" void U2Gate(double phi, double lam, qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _U2Gate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 2; 
  qcs_info[thread_id].gate[n].rarg[0] = phi;
  qcs_info[thread_id].gate[n].rarg[1] = lam;
  qcs_info[thread_id].ngates++;
}

extern "C" void U3Gate(double theta, double phi, double lam, qint target_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _U3Gate;
  qcs_info[thread_id].gate[n].niarg   = 1; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 3; 
  qcs_info[thread_id].gate[n].rarg[0] = phi;
  qcs_info[thread_id].gate[n].rarg[1] = lam;
  qcs_info[thread_id].gate[n].rarg[0] = theta;
  qcs_info[thread_id].gate[n].rarg[1] = phi;
  qcs_info[thread_id].gate[n].rarg[2] = lam;
  qcs_info[thread_id].ngates++;
}

extern "C" void CRXGate(double theta, qint target_qubit, qint control_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _CRXGate;
  qcs_info[thread_id].gate[n].niarg   = 2; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].iarg[1] = control_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 1; 
  qcs_info[thread_id].gate[n].rarg[0] = theta;
  qcs_info[thread_id].ngates++;
}

extern "C" void CRYGate(double theta, qint target_qubit, qint control_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _CRYGate;
  qcs_info[thread_id].gate[n].niarg   = 2; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].iarg[1] = control_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 1; 
  qcs_info[thread_id].gate[n].rarg[0] = theta;
  qcs_info[thread_id].ngates++;
}

extern "C" void CRZGate(double theta, qint target_qubit, qint control_qubit)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _CRZGate;
  qcs_info[thread_id].gate[n].niarg   = 2; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].iarg[1] = control_qubit;
  qcs_info[thread_id].gate[n].nrarg   = 1; 
  qcs_info[thread_id].gate[n].rarg[0] = theta;
  qcs_info[thread_id].ngates++;
}

extern "C" void CCXGate(qint target_qubit, qint control_qubit0, qint control_qubit1)
{
  int n = qcs_info[thread_id].ngates; 
  QC_n_gate_check(n);
  qcs_info[thread_id].gate[n].id      = _CCXGate;
  qcs_info[thread_id].gate[n].niarg   = 3; 
  qcs_info[thread_id].gate[n].iarg[0] = target_qubit;
  qcs_info[thread_id].gate[n].iarg[1] = control_qubit0;
  qcs_info[thread_id].gate[n].iarg[2] = control_qubit1;
  qcs_info[thread_id].gate[n].nrarg   = 0; 
  qcs_info[thread_id].ngates++;
}
