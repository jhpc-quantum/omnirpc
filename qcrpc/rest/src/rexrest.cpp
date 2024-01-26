#include "rexrest.h"
#include "rqc_rest.hpp"

int calculate(uint32_t rex_type, const char *base_url, const char *token, const char *qasm,
              uint32_t shots, uint32_t transpiler, const char *remark,
              uint32_t polling_interval, uint32_t max_polling_count,
              char **output, size_t *output_len) {
    if (rex_type == 0) {
        std::shared_ptr< rexrest::JobResult> result =
            rexrest::RQCClient::Calculate(base_url, token, qasm, shots, transpiler, remark,
                                          polling_interval, max_polling_count);
        if (result->GetStatusCode() == web::http::status_codes::OK) {
            size_t result_len = result->GetJsonBody().length() + 1;
            if (*output == NULL) {
                *output = (char *)malloc(sizeof(char) * result_len);
                if (*output == NULL) {
                    fprintf(stderr, "Failed to malloc.\n");
                    // TODO
                    return -1;
                }
            }
            int len = snprintf(*output, result_len, result->GetJsonBody().c_str());
            if (len < 0) {
                fprintf(stderr, "Failed to output results.\n");
                // TODO
                return -1;
            }

            *output_len = len;

            return 0;
        } else {
            fprintf(stderr, "Failed to calculate.\n");
            // TODO
            return -1;
        }
    }

    return -1;
}

