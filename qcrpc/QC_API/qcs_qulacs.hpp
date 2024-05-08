#pragma once

#include "qcs_types.h"

#include <iostream>
#include <cppsim/state.hpp>
#include <cppsim/circuit.hpp>
#include <cppsim/observable.hpp>
#include <cppsim/gate_factory.hpp>
#include <cppsim/gate_merge.hpp>
#include <cppsim/circuit_optimizer.hpp>

// --- qulacs parameters ---
typedef struct {
  qint              nqubits;
  QuantumCircuit    *circuit;
  QuantumStateCpu   *st;
} qulacs_info_t;


__BEGIN_DECLS


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
