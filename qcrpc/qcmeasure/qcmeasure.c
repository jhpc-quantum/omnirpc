#include "omni_platform.h"
#include "qcs_api.hpp"

int
main(int argc, char *argv[]) {
  if (likely(argc > 2)) {
    int nqubits = atoi(argv[1]);
    char *file = argv[2];

    if (likely(nqubits > 0 && file != NULL && *file != '\0')) {
      QC_Init(&argc, &argv, nqubits, 0);
      QC_LoadContext(file);
      QC_Measure();
      QC_Finalize();
      return 0;
    }
  }

  return 1;
}

