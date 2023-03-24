#include "omni_platform.h"
#include "qcs_types.h"
#include "qcs_api.hpp"
#include "qcs_qulacs.hpp"

static qcs_info_t qcs_info[MAX_INFO];
static ssize_t n_infos = 0;
static pthread_key_t idx_key;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static bool s_is_rpc_inited = false;

static void once_proc(void) {
  int st = pthread_key_create(&idx_key, NULL);
  if (unlikely(st != 0)) {
    fprintf(stderr, "Error: can't crete info index key.\n");
    exit(1);
  }
}

static inline qcs_info_t * QC_check_ninfo_ngate(void)
{
  int n;
  int st;
  qcs_info_t *ret = (qcs_info_t *)pthread_getspecific(idx_key);
  if (likely(ret >= &qcs_info[0] && ret < &qcs_info[MAX_INFO - 1] &&
             (n = ret->ngates) < MAX_INFO)) {
    return ret;
  } else {
    if (ret == 0) {
      fprintf(stderr, "Error: invalid qcs_info index.\n");
    }
    exit(1);
    /* not reached */
    return NULL;
  }
}

void QC_Init(int *argc, char ***argv, qint qubits, int qcs_id)
{
  int st;
  ssize_t idx;
  if (likely(qcs_id < _Nsims && qcs_id >= 0 /* qulacs == 0 */ &&
             (idx = __atomic_fetch_add(&n_infos, 1, __ATOMIC_ACQ_REL)) >= 0 &&
             idx < MAX_INFO && n_infos > 0)) {
    (void)pthread_once(&once, once_proc);
    st = pthread_setspecific(idx_key, &qcs_info[idx]);
    if (likely(st == 0)) {
      // 必要な部分を初期化 （ここでは仮にqulacsを用いる初期化）
      qcs_info[idx].qcs_id  = qcs_id;
      qcs_info[idx].qubits  = qubits;
      qcs_info[idx].ngates  = 0; 
      qcs_info[idx].nprocs  = 1;
      return;
    } else {
      fprintf(stderr, "Error: can't set info index.\n");
      exit(1);
    }
  } else {
    if (idx >= MAX_INFO) {
      fprintf(stderr, "Error: too many info, < %d.\n", MAX_INFO);
    } else if (qcs_id >= _Nsims || qcs_id < 0) {
      fprintf(stderr, "Error: simultion id number is invalid\n");
    }
    exit(1);
  }
}

void QC_InitRemote(int *argc, char **argv[])
{
  return;
}

void QC_SetNodes(int nprocs)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  info->nprocs = nprocs;
}

void QC_Finalize(void)
{
  //qcs_info_t *info = QC_check_ninfo_ngate();
}

void QC_Measure(void)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  qcs_measure(info);
}

void IGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _IGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void XGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _XGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void ZGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _ZGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void HGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _HGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void SGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _SGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void SdgGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _SdgGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void TGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _TGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void TdgGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _TdgGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void SXGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _SXGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void SXdgGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _SXdgGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void SYGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _SYGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void SYdgGate(qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _SYdgGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void CXGate(qint target_qubit, qint control_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _CXGate;
  info->gate[n].niarg   = 2; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].iarg[1] = control_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void CYGate(qint target_qubit, qint control_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _CYGate;
  info->gate[n].niarg   = 2; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].iarg[1] = control_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void CZGate(qint target_qubit, qint control_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _CZGate;
  info->gate[n].niarg   = 2; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].iarg[1] = control_qubit;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void SwapGate(qint target_qubit0, qint target_qubit1)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _SwapGate;
  info->gate[n].niarg   = 2; 
  info->gate[n].iarg[0] = target_qubit0;
  info->gate[n].iarg[1] = target_qubit1;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}

void RXGate(double theta, qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _RXGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 1; 
  info->gate[n].rarg[0] = theta;
  info->ngates++;
}

void RYGate(double theta, qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _RYGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 1; 
  info->gate[n].rarg[0] = theta;
  info->ngates++;
}

void RZGate(double theta, qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _RZGate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 1; 
  info->gate[n].rarg[0] = theta;
  info->ngates++;
}

void U1Gate(double theta, qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _U1Gate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 1; 
  info->gate[n].rarg[0] = theta;
  info->ngates++;
}

void U2Gate(double phi, double lam, qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _U2Gate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 2; 
  info->gate[n].rarg[0] = phi;
  info->gate[n].rarg[1] = lam;
  info->ngates++;
}

void U3Gate(double theta, double phi, double lam, qint target_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _U3Gate;
  info->gate[n].niarg   = 1; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].nrarg   = 3; 
  info->gate[n].rarg[0] = phi;
  info->gate[n].rarg[1] = lam;
  info->gate[n].rarg[0] = theta;
  info->gate[n].rarg[1] = phi;
  info->gate[n].rarg[2] = lam;
  info->ngates++;
}

void CRXGate(double theta, qint target_qubit, qint control_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _CRXGate;
  info->gate[n].niarg   = 2; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].iarg[1] = control_qubit;
  info->gate[n].nrarg   = 1; 
  info->gate[n].rarg[0] = theta;
  info->ngates++;
}

void CRYGate(double theta, qint target_qubit, qint control_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _CRYGate;
  info->gate[n].niarg   = 2; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].iarg[1] = control_qubit;
  info->gate[n].nrarg   = 1; 
  info->gate[n].rarg[0] = theta;
  info->ngates++;
}

void CRZGate(double theta, qint target_qubit, qint control_qubit)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _CRZGate;
  info->gate[n].niarg   = 2; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].iarg[1] = control_qubit;
  info->gate[n].nrarg   = 1; 
  info->gate[n].rarg[0] = theta;
  info->ngates++;
}

void CCXGate(qint target_qubit, qint control_qubit0, qint control_qubit1)
{
  qcs_info_t *info = QC_check_ninfo_ngate();
  int n = info->ngates; 
  info->gate[n].id      = _CCXGate;
  info->gate[n].niarg   = 3; 
  info->gate[n].iarg[0] = target_qubit;
  info->gate[n].iarg[1] = control_qubit0;
  info->gate[n].iarg[2] = control_qubit1;
  info->gate[n].nrarg   = 0; 
  info->ngates++;
}
