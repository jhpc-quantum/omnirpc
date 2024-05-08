#ifndef REXREST_H_
#define REXREST_H_

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
int calculate(uint32_t qc_type, const char *base_url, const char *token, const char *qasm,
              uint32_t shots, uint32_t transpiler, const char *remark,
              uint32_t polling_interval, uint32_t max_polling_count,
              char **output, size_t *output_len);

int scrape_response(uint32_t qc_type, const char *resp, uint32_t shots, int **patterns,
                    float **probs, size_t *n_patterns);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // REXREST_H_

