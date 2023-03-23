// test for qulacs

#include <iostream>
#include <cppsim/state.hpp>
#include <cppsim/circuit.hpp>
#include <cppsim/observable.hpp>
#include <cppsim/gate_factory.hpp>
#include <cppsim/gate_merge.hpp>
#include <cppsim/circuit_optimizer.hpp>

#include <stdio.h>
#include <stdio.h>
#include <sys/time.h>

double gettimeofday_sec()
{
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return tv.tv_sec + tv.tv_usec * 1e-6;
}

double drand()
{
  return (double)random()/(double)RAND_MAX;
}

int func(int nqubits){
  double et0 = gettimeofday_sec();
  int depth   = 9;
  QuantumCircuit          circuit(nqubits);
  //QuantumCircuitOptimizer qco;
  QuantumState            st(nqubits);
  circuit.add_X_gate(2);
  circuit.add_X_gate(1);
  circuit.add_X_gate(0);
  circuit.add_H_gate(0);
  circuit.add_Sdag_gate(0);
  //circuit.add_U1_gate(0, 0.1);
  //circuit.add_U2_gate(1, 0.1,0.2);
  //circuit.add_U3_gate(2,0.1,0.2,0.3);
  
  //U1Gate(0, 0.1);
  //U2Gate(1, 0.1, 0.2);
  //U3Gate(1, 0.1, 0.2, 0.3);


  //qco.optimize_light(&circuit);
  circuit.update_quantum_state(&st);

  // (sec)
  std::cout << nqubits << " " << (gettimeofday_sec()-et0) << " (sec)" <<std::endl;

  const CPPCTYPE* raw_data_cpp = st.data_cpp();
  for(int i=0; i<8; i++){
    std:: cout << raw_data_cpp[i] << std::endl;
  }

  return 0;
}

int main(int argc, char **argv)
{
  for(int nqubits=3; nqubits<=3; nqubits++){
    func(nqubits);
  }
}
