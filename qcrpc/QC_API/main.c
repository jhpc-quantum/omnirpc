// Test without OmniRPC

#include<stdio.h>
#include"qcs_api.hpp"

int main(int argc, char **argv)
{
  int qubits = 5;
  QC_Init(&argc, &argv, qubits, 0); // 0 = qulacs

  HGate(0);
  U1Gate(0.1, 0);
  U2Gate(0.1, 0.2, 1);
  U3Gate(0.1, 0.2, 0.3, 2);

  QC_Measure();

  //QC_Finalize();

}

