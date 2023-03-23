#pragma once

#define MAX_N_GATES 128 
#define MAX_I_ARGS  8
#define MAX_R_ARGS  8

typedef int qint;

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
} gate_info;

typedef struct{
  // --- common parameters --- 
  int            qcs_id;
  int            nprocs;
  qint           qubits;
  int            ngates; 
  gate_info      gate[MAX_N_GATES]; 
} qcs_info_t;

