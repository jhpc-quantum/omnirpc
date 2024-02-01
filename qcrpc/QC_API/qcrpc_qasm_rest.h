#pragma once

static inline void
s_submit_qasm_rest(const char *url,
		   const char *token,
		   const char *qasm,
		   int qc_type,
		   const char *rem,
		   int shots,
		   int poll_ms,
		   int poll_max,
		   int transpiler,
		   char *out,
		   int maxout,
		   int *olen) {
  int ulen = 0;
  int tlen = 0;
  int qlen = 0;
  int rlen = 0;
  char remark[65536];

  if ((url != NULL && (ulen = (strlen(url) + 1)) > 1) &&
      (token != NULL && (tlen = (strlen(token) + 1)) > 1) &&
      (qasm != NULL && (qlen = (strlen(qasm) + 1)) > 1) &&
      (qc_type >= 0) &&
      (shots > 0 && poll_ms > 0 && poll_max >= 0) &&
      (transpiler >= 0) &&
      (out != NULL && maxout > 1) &&
      (olen != NULL)) {
    int ret = -INT_MAX;
    int getlen = -INT_MAX;

    if (rem != NULL && (rlen = strlen(rem)) > 0) {
      snprintf(remark, sizeof(remark), "%s", rem);
      rlen += 1;
    } else {
      remark[0] = '\0';
      rlen = 0;
    }

    fprintf(stderr, "debug: rem %d, '%s'\n", rlen, rem);

    *olen = -INT_MAX;
    OmniRpcRequest r =
        OmniRpcCallAsync("qc_rpc_rest_qasm_string",
                         /* 00, 01 */ ulen, url,
                         /* 02, 03 */ tlen, token,
                         /* 04, 05 */ qlen, qasm,
                         /* 06     */ qc_type,
                         /* 07, 08 */ rlen, remark,
                         /* 09     */ shots,
                         /* 10     */ poll_ms,
                         /* 11     */ poll_max,
                         /* 12     */ transpiler,

                         /* 13     */ &ret,
                         /* 14, 15 */ maxout, out,
                         /* 16     */ &getlen);
    OmniRpcWait(r);

    fprintf(stderr, "debug: ret %d, getlen %d\n", ret, getlen);
    if (ret >= 0) {
      *olen = getlen;
    }
  }
}
