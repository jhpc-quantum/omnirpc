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
