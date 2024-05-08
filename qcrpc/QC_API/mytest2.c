// test for braket
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

int main()
{
  int  argc    = 0;
  char **argv  = NULL;
  auto environment  = new yampi::environment(argc, argv, yampi::thread_support::single);
  auto communicator = new yampi::communicator(yampi::tags::world_communicator());
  auto rank         = communicator->rank(*(environment));
  auto nprocs       = communicator->size(*(environment));

  auto const num_gqubits = ket::utility::integer_log2<bit_integer_type>(nprocs);
  auto const nqubits = 5;
  auto const num_qubits = bit_integer_type{(unsigned int)nqubits};
  auto const num_lqubits = num_qubits - num_gqubits;
  //auto const initial_state_value = state_integer_type{(1u<<num_qubits)-1};  
  auto const initial_state_value = state_integer_type{0u};

  auto permutation = new ket::mpi::qubit_permutation<state_integer_type, bit_integer_type>{num_qubits};
  auto local_state = new ket::mpi::state<complex_type, false, yampi::allocator<complex_type>>{num_lqubits, initial_state_value, *(permutation), *(communicator), *(environment)}; 
  auto buffer = std::vector<complex_type>{};
  

  qubit_type target_qubit{bit_integer_type{(unsigned int)(0)}};
  ket::control<qubit_type> control_qubit0{qubit_type{bit_integer_type{(unsigned int)(1)}}};
  ket::control<qubit_type> control_qubit1{qubit_type{bit_integer_type{(unsigned int)(2)}}};
  ket::mpi::gate::toffoli(*(local_state), target_qubit, control_qubit0, control_qubit1, *(permutation), buffer, *(communicator), *(environment));

  
}
