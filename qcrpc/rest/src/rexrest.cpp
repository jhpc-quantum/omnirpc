#include "rexrest.h"
#include "rqc_rest.hpp"
#include "rqc_json.hpp"
#include "ibmq_rest.hpp"
#include "ibmq_json.hpp"

int calculate(uint32_t qc_type, const char *base_url, const char *token, const char *qasm,
              uint32_t shots, uint32_t transpiler, const char *remark,
              uint32_t polling_interval, uint32_t max_polling_count,
              char **output, size_t *output_len) {
    std::shared_ptr<rexrest::JobResult> result;
    if (qc_type == 0) {
        result = rexrest::RQCClient::Calculate(base_url, token, qasm, shots, transpiler, remark,
                                               polling_interval, max_polling_count);
    } else if (qc_type == 1) {
        result = rexrest::IBMQClient::Calculate(base_url, token, qasm, shots, transpiler, remark,
                                                polling_interval, max_polling_count);
    } else {
        fprintf(stderr, "Unsupported QC type.\n");
        return -1;
    }

    if (result->GetStatusCode() == web::http::status_codes::OK) {
        size_t result_len = result->GetJsonBody().length() + 1;
        bool allocated = false;
        if (*output == NULL) {
            *output = (char *)malloc(sizeof(char) * result_len);
            if (*output == NULL) {
                fprintf(stderr, "Failed to malloc.\n");
                return -1;
            }
            allocated = true;
        }

        int len = snprintf(*output, result_len, result->GetJsonBody().c_str());
        if (len < 0) {
            if (allocated == true) {
                free(*output);
                *output = NULL;
            }
            fprintf(stderr, "Failed to output results.\n");
            return -1;
        }

        *output_len = len;

        return 0;
    } else {
        fprintf(stderr, "Failed to calculate.\n");
        return -1;
    }
}

int scrape_response(uint32_t qc_type, const char *resp, uint32_t shots, int **patterns,
                    float **probs, size_t *n_patterns) {
    std::shared_ptr<rexrest::JsonResult> result;
    try {
        if (qc_type == 0) {
            result = rexrest::RQCJsonParser::ScrapeResponse(resp, shots);
        } else if (qc_type == 1) {
            result = rexrest::IBMQJsonParser::ScrapeResponse(resp, shots);
        } else {
            fprintf(stderr, "Unsupported QC type.\n");
            return -1;
        }
    } catch (const std::exception& e) {
        std::wcout << "JSON Parse Error: " << e.what() << std::endl;
        return -1;
    }

    std::uint32_t resultSize = result->GetSize();

    if (*patterns == NULL && *probs == NULL) {
        *patterns = (int *)malloc(sizeof(int) * resultSize);
        if (*patterns == NULL) {
            fprintf(stderr, "Failed to malloc.\n");
            return -1;
        }

        *probs = (float *)malloc(sizeof(float) * resultSize);
        if (*probs == NULL) {
            fprintf(stderr, "Failed to malloc.\n");
            return -1;
        }
    } else if (*patterns != NULL && *probs != NULL) {
        resultSize = std::min(shots, resultSize);
    } else {
        fprintf(stderr, "Invalid argument.\n");
        return -1;
    }

    std::vector<int> jsonPatterns = result->GetPatterns();
    std::vector<float> jsonProbs = result->GetProbs();

    for (int i = 0; i < resultSize; i++) {
        (*patterns)[i] = jsonPatterns[i];
        (*probs)[i] = jsonProbs[i];
    }

    *n_patterns = resultSize;

    return 0;
}

