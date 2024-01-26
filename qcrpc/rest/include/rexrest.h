#ifndef REXREST_H_
#define REXREST_H_

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
int calculate(uint32_t rex_type, const char *base_url, const char *token, const char *qasm,
              uint32_t shots, uint32_t transpiler, const char *remark,
              uint32_t polling_interval, uint32_t max_polling_count,
              char **output, size_t *output_len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // REXREST_H_

