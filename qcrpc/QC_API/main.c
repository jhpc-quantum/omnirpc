#include "omni_platform.h"
#include "qcs_api.hpp"

int main(int argc, char *argv[])
{
  int qubits = 5;
  bool do_save = false;
  bool do_load = false;
  bool do_rpc = false;
  bool do_job_submit = false;
  
  if (argc == 2 && strcmp(argv[1], "-s") == 0) {
    do_save = true;
  } else if (argc == 2 && strcmp(argv[1], "-l") == 0) {
    do_load = true;
  } else if (argc == 2 && strcmp(argv[1], "-re") == 0) {
    do_rpc = true;
  } else if (argc == 2 && strcmp(argv[1], "-rs") == 0) {
    do_job_submit = true;
    do_rpc = true;
  }

  if (do_rpc == true) {
    QC_InitRemote(&argc, &argv);
  }
  
  QC_Init(&argc, &argv, qubits, 0); // 0 = qulacs

  if (do_load == false) {

    HGate(0);
    U1Gate(0.1, 0);
    U2Gate(0.1, 0.2, 1);
    U3Gate(0.1, 0.2, 0.3, 2);

    if (do_save == true) {
      QC_SaveContext("test.dump");
    } else if (do_rpc == true) {
      QC_MeasureRemote(do_job_submit);
    }
  } else {
    QC_LoadContext("test.dump");
  }

  if (do_save == false && do_rpc == false) {
    QC_Measure();
  }

  QC_Finalize();
}

