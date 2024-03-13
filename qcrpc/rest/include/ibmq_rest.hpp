#ifndef IBMQ_REST_HPP_
#define IBMQ_REST_HPP_

#include "rexrest_common.hpp"

namespace rexrest {

class IBMQClient {
public:
    static std::shared_ptr<JobResult> Calculate(const std::string& base_url, const std::string& token,
                                                const std::string& qasm, const std::uint32_t shots,
                                                const std::uint32_t transpiler, const std::string& remark) {
        return Calculate(base_url, token, qasm, shots, transpiler, remark, kPollingInterval, kMaxPollingCount);
    }

    static std::shared_ptr<JobResult> Calculate(const std::string& base_url, const std::string& token,
                                                const std::string& qasm, const std::uint32_t shots,
                                                const std::uint32_t transpiler, const std::string& remark,
                                                std::uint32_t polling_interval, std::uint32_t max_polling_count);

private:
    static const std::uint32_t kPollingInterval;
    static const std::uint32_t kMaxPollingCount;
};

enum class IBMQTranspiler : std::uint32_t {
    NONE = 0,
};

inline const std::string IBMQTranspilerToString(IBMQTranspiler transpiler) {
  switch (transpiler) {
    case IBMQTranspiler::NONE: { return "none"; }
    default: { return "none"; }
  }
}

class IBMQHttpClient {
public:
    static const std::string kListJobDetailsPath;
    static const std::string kListJobResultsPath;
    static const std::string kRunJobPath;
    static const std::string kIDKey;
    static const std::string kStatusKey;
    static const std::string kStatusCompletedValue;
    static const std::string kStatusFailedValue;
    static const std::string kStatusCancelledValue;
    static const std::string kStateKey;
    static const std::string kReasonKey;

    static std::shared_ptr<IBMQHttpClient> CreateHttpClient(const std::string& base_url);

    IBMQHttpClient(const std::string& base_url) : base_url_(base_url), http_client_(base_url) {}
    ~IBMQHttpClient() {}

    std::shared_ptr<JobResult> ListJobDetails(const std::string& path, const std::string& token, const std::string& job_id);
    std::shared_ptr<JobResult> ListJobResults(const std::string& path, const std::string& token, const std::string& job_id);
    std::shared_ptr<JobResult> RunJob(const std::string& path, const std::string& token, const std::string& qasm,
                                      const std::uint32_t shots, const std::string& transpiler, const std::string& remark);
    std::shared_ptr<JobResult> CancelJob(const std::string& path, const std::string& token, const std::string& job_id);
    std::shared_ptr<JobResult> DeleteJob(const std::string& path, const std::string& token, const std::string& job_id);

private:
    static const std::string kAuthorizationHeader;
    static const std::string kContentTypeHeader;

    std::string base_url_;
    web::http::client::http_client http_client_;
};

}  // namespace rexrest

#endif  // IBMQ_REST_HPP_

