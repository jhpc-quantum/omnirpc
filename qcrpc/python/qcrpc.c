#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "omni_platform.h"
#include "OmniRpc.h"

#include "qcrpc_qasm_rest.h"

static bool is_inited = false;
static const char *submit_qasm_kwlist[] = {
  "url",
  "token",
  "qasm",
  "type",
  "remark",
  "shots",
  "poll_interval",
  "poll_count_max",
  "transpile",
  NULL
};



static PyObject *
qcrpc_submit_qasm(PyObject *self, PyObject *args, PyObject *kwargs) {
  PyObject *ret = NULL;
  char *url = NULL;
  char *token = NULL;
  char *qasm = NULL;
  int qc_type = 0;
  int tmp_qc_type = -INT_MAX;
  char *rem = NULL;
  int shots = 100;
  int tmp_shots = -INT_MAX;
  int poll_ms = 1000;
  int tmp_poll_ms = -INT_MAX;
  int poll_max = 10;
  int tmp_poll_max = -INT_MAX;
  int transpile = 0;
  int tmp_transpile = 0;

  (void)self;

  if (PyArg_ParseTupleAndKeywords(args, kwargs, "sssi|siiii",
                                  submit_qasm_kwlist,
                                  &url,
                                  &token,
                                  &qasm,
                                  &tmp_qc_type,
                                  &rem,
                                  &tmp_shots,
                                  &tmp_poll_ms,
                                  &tmp_poll_max,
                                  &tmp_transpile)) {
#define MAX_RESPONSE_SIZE	32 * 1024 * 1024	/* 32MiB */
    int retlen = -INT_MAX;

    if ((url != NULL && *url != '\0') &&
        (token != NULL && *token != '\0') &&
        (qasm != NULL && *qasm != '\0') &&
        tmp_qc_type >= 0) {
      char *retbuf = (char *)malloc(sizeof(char) * MAX_RESPONSE_SIZE);
      if (retbuf != NULL) {
        if (tmp_shots != -INT_MAX) {
          shots = tmp_shots;
        }
        if (tmp_poll_ms != -INT_MAX) {
          poll_ms = tmp_poll_ms;
        }
        if (tmp_poll_max != -INT_MAX) {
          poll_max = tmp_poll_max;
        }
        if (tmp_transpile != -INT_MAX) {
          transpile = tmp_transpile;
        }
        qc_type = tmp_qc_type;

        s_submit_qasm_rest(url,
                           token,
                           qasm,
                           qc_type,
                           rem,
                           shots,
                           poll_ms,
                           poll_max,
                           transpile,
                           retbuf,
                           sizeof(char) * MAX_RESPONSE_SIZE,
                           &retlen);
        if (retlen > 0 && *retbuf != '\0') {
          retbuf[retlen] = '\0';
          ret = PyUnicode_FromString(retbuf);
        }
        free(retbuf);
      }
    }
  }

  return ret;
}

static PyMethodDef qcrpc_methods[] = {
  { "submit_qasm",
    (PyCFunction)qcrpc_submit_qasm,
    METH_VARARGS | METH_KEYWORDS,
    "Submit a QASM file measurement job at the specified URL "
    "with and the API token." },
  { NULL, NULL, 0, NULL }
};

static struct PyModuleDef qcrpc_module = {
  PyModuleDef_HEAD_INIT,
  "qcrpc",
  NULL,
  -1,

  qcrpc_methods
};

PyMODINIT_FUNC
PyInit_qcrpc(void) {
  if (is_inited != true) {
    OmniRpcInit(NULL, NULL);
    is_inited = true;
  }
  return PyModule_Create(&qcrpc_module);
}

