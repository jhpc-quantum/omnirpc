#include "omni_platform.h"
#include "qcs_api.hpp"

int main(int argc, char *argv[])
{
  int qubits = 5;
  bool do_save = false;
  bool do_load = false;
  bool do_rpc = false;
  bool do_job_submit = false;
  bool do_qasm = false;
  char *file = NULL;
  char *dir = NULL;

  if (argc == 4 && strcmp(argv[1], "-rq") == 0) {
    do_rpc = true;
    do_qasm = true;
    dir = strdup(argv[2]);
    file = strdup(argv[3]);
  } else if (argc == 2 && strcmp(argv[1], "-s") == 0) {
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

  if (do_qasm == true) {
    if (dir != NULL && *dir != '\0' &&
        dir != NULL && *dir != '\0') {
      int max_shots = 1000;
      int idx[max_shots];
      double n_cnts[max_shots];
      int n_shots = 0;
      int i;

      QC_MeasureRemoteQASM(dir, file, idx, n_cnts, max_shots, &n_shots);

      for (i = 0; i < n_shots; i++) {
        fprintf(stdout, "%4d: %4d\t%8.4f\n", i, idx[i], n_cnts[i]);
      }
      
    } else {
      fprintf(stderr, "error: -rq flag requires work directory and "
              "QASM filename under the work directory.\n");
    }
    goto done;
  }
      
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

done:
  QC_Finalize();

  return 0;
}

