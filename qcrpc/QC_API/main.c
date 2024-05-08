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
  bool do_rest = false;
  char *file = NULL;
  char *dir = NULL;

  if (argc == 2 && strcmp(argv[1], "-s") == 0) {
    do_save = true;
  } else if (argc == 2 && strcmp(argv[1], "-l") == 0) {
    do_load = true;
  } else if (argc == 2 && strcmp(argv[1], "-re") == 0) {
    do_rpc = true;
  } else if (argc == 2 && strcmp(argv[1], "-rs") == 0) {
    do_job_submit = true;
    do_rpc = true;
  } else if (argc == 4 && strcmp(argv[1], "-rq") == 0) {
    do_rpc = true;
    do_qasm = true;
    dir = strdup(argv[2]);
    file = strdup(argv[3]);
  } else if (strcmp(argv[1], "-rest") == 0) {
    do_rpc = true;
    do_qasm = true;
    do_rest = true;
  }

  if (do_rpc == true) {
    QC_InitRemote(&argc, &argv);
  }
  
  QC_Init(&argc, &argv, qubits, 0); // 0 = qulacs

  if (do_qasm == false) {
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
  } else {
    if (do_rest == true) {
#ifdef USE_REST
#undef SHOTS
#define SHOTS	1024
      char *url = NULL;
      char *default_qasm =
          "OPENQASM 3; include \"stdgates.inc\"; "
          "qreg q[4]; creg c[4]; "
          "h q[0]; h q[1]; ccx q[0], q[1], q[2]; cx q[0], q[3]; cx q[1],q[3]; "
          "measure q[3] -> c[0]; measure q[2] -> c[1]; "
          "measure q[1] -> c[2]; measure q[0] -> c[3]; ";
      char *qasm = NULL;
      char *token = NULL;
      char *rem = NULL;
      int qc_type = 0;
      int shots = SHOTS;
      int poll_ms = 1000;
      int poll_max = 180;
      int transpiler = 2;
      int pattern[SHOTS];
      float count[SHOTS];
      int n_patterns = -INT_MAX;
      int i;
      int tmp;

      for (i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-url") == 0) {
          i++;
          url = argv[i];
        } else if (strcmp(argv[i], "-token") == 0) {
          i++;
          token = argv[i];
        } else if (strcmp(argv[i], "-qasm") == 0) {
          i++;
          qasm = argv[i];
	} else if (strcmp(argv[i], "-qctype") == 0) {
          i++;
          tmp = atoi(argv[i]);
          if (tmp >= 0) {
            qc_type = tmp;
          }
        } else if (strcmp(argv[i], "-rem") == 0) {
          i++;
          rem = argv[i];
        } else if (strcmp(argv[i], "-shots") == 0) {
          i++;
          tmp = atoi(argv[i]);
          if (tmp < 1024 && tmp > 0) {
            shots = tmp;
          }
        } else if (strcmp(argv[i], "-poll-interval") == 0) {
          i++;
          tmp = atoi(argv[i]);
          if (tmp > 0) {
            poll_ms = tmp;
          }
        } else if (strcmp(argv[i], "-poll-max") == 0) {
          i++;
          tmp = atoi(argv[i]);
          if (tmp > 0) {
            poll_max = tmp;
          }
        } else if (strcmp(argv[i], "-transpiler") == 0) {
          i++;
          tmp = atoi(argv[i]);
          if (tmp >= 0) {
            transpiler = tmp;
          }
        }
      }

      if (qasm == NULL) {
        qasm = default_qasm;
      }

      if (url != NULL && *url != '\0' &&
          token != NULL && *token != '\0' &&
          qasm != NULL && *qasm != '\0') {
        QC_MeasureRemoteQASMStringRESTArray(url, token, qasm, qc_type, rem,
                                            shots, poll_ms, poll_max,
                                            transpiler,
                                            pattern, count, &n_patterns);
        if (n_patterns > 0) {
          for (i = 0; i < n_patterns; i++) {
            fprintf(stdout, "%5d,\t%f\n", pattern[i], count[i]);
          }
        } else {
          fprintf(stderr, "error: invalid result or any failures.\n");
        }
      } else {
        fprintf(stderr, "error: -rest flags requires at leaset "
                "-token option.\n");
      }
#undef SHOTS
#else
      fprintf(stderr, "the REST support not enabled.\n");
#endif /* USE_REST */
    } else if (dir != NULL && *dir != '\0' &&
               dir != NULL && *dir != '\0') {
      int max_shots = 1000;
      int idx[max_shots];
      double n_cnts[max_shots];
      int n_shots = 0;
      int i;

      QC_MeasureRemoteQASMFile(dir, file,
			       idx, n_cnts, max_shots, &n_shots);

      for (i = 0; i < n_shots; i++) {
        fprintf(stdout, "%4d: %4d\t%8.4f\n", i, idx[i], n_cnts[i]);
      }
    } else {
      fprintf(stderr, "error: -rq flag requires work directory and "
              "QASM filename under the work directory.\n");
    }
  }

  QC_Finalize();

  return 0;
}

