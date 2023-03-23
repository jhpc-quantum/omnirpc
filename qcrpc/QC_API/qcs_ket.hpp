#ifndef _QCS_BRA_H_
#define _QCS_BRA_H_

#include "type.h"

#include <iostream>
#include <vector>
#include <random>
#include <omp.h>
#include <stdio.h>

#include <ket/qubit.hpp>
#include <ket/utility/integer_log2.hpp>
#include <ket/utility/parallel/loop_n.hpp>
#include <ket/mpi/state.hpp>
#include <ket/mpi/qubit_permutation.hpp>
#include <ket/mpi/gate/controlled_not.hpp>
#include <ket/mpi/gate/hadamard.hpp>
#include <ket/mpi/gate/pauli_x.hpp>
#include <ket/mpi/gate/pauli_y.hpp>
#include <ket/mpi/gate/pauli_z.hpp>
#include <ket/mpi/gate/phase_shift.hpp>
#include <ket/mpi/gate/controlled_phase_shift.hpp>
#include <ket/mpi/gate/x_rotation_half_pi.hpp>
#include <ket/mpi/gate/y_rotation_half_pi.hpp>
#include <ket/mpi/gate/toffoli.hpp>
#include <yampi/allocator.hpp>
#include <yampi/rank.hpp>
#include <yampi/communicator.hpp>
#include <yampi/environment.hpp>
#include "mult.hpp"

using bit_integer_type      = unsigned int;
using state_integer_type    = std::uint64_t;
using complex_type          = std::complex<double>;
using state_integer_type    = std::uint64_t;
using qubit_type            = ket::qubit<state_integer_type, bit_integer_type>;  
using permutated_qubit_type = ket::mpi::permutated<qubit_type>;


class ket_info{
public:
  qint              nqubits;
  yampi::environment  *environment;
  yampi::communicator *communicator;
  yampi::rank rank;
  yampi::rank root;
  ket::mpi::state<complex_type, false, yampi::allocator<complex_type>> *local_state;
  ket::mpi::qubit_permutation<state_integer_type, bit_integer_type> *permutation;
  int nprocs;
  int myrank;
};


#endif //_QCS_BRA_H_
