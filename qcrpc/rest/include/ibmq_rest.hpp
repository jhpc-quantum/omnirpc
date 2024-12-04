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
    NOAI = 1,
    AI = 2
};

inline const std::string IBMQTranspilerUseAI(IBMQTranspiler transpiler) {
    switch (transpiler) {
        case IBMQTranspiler::AI: { return "true"; }
        default: { return "false"; }
    }
}

class IBMQHttpClient {
public:
    static const std::string kTranspilerServiceURL;

    static const std::string kBackend;
    static const std::string kHub;
    static const std::string kGroup;
    static const std::string kProject;

    static const std::string kTranspilePath;
    static const std::string kGetTranspilationResultsPath;
    static const std::string kListJobDetailsPath;
    static const std::string kListJobResultsPath;
    static const std::string kRunJobPath;

    static const std::string kIDKey;
    static const std::string kTaskIDKey;
    static const std::string kStatusKey;
    static const std::string kStateKey;
    static const std::string kReasonKey;
    static const std::string kResultKey;
    static const std::string kQASMKey;

    static const std::string kStatusCompletedValue;
    static const std::string kStatusFailedValue;
    static const std::string kStatusCancelledValue;
    static const std::string kStateSuccessValue;
    static const std::string kStateFailureValue;

    static std::shared_ptr<IBMQHttpClient> CreateHttpClient(const std::string& runtime_url);

    IBMQHttpClient(const std::string& runtime_url) : IBMQHttpClient(runtime_url, kTranspilerServiceURL) {}
    IBMQHttpClient(const std::string& runtime_url, const std::string& transpile_url) : runtime_url_(runtime_url), transpile_url_(transpile_url),
        runtime_http_client_(runtime_url), transpile_http_client_(transpile_url) {}
    ~IBMQHttpClient() {}

    std::shared_ptr<JobResult> TranspileCircuit(const std::string& token, const std::string& qasm, const IBMQTranspiler transpiler);
    std::shared_ptr<JobResult> GetTranspilationResults(const std::string& token, const std::string& task_id);
    std::shared_ptr<JobResult> ListJobDetails(const std::string& path, const std::string& token, const std::string& job_id);
    std::shared_ptr<JobResult> ListJobResults(const std::string& path, const std::string& token, const std::string& job_id);
    std::shared_ptr<JobResult> RunJob(const std::string& path, const std::string& token, const std::string& qasm,
                                      const std::uint32_t shots, const std::string& remark);
    std::shared_ptr<JobResult> CancelJob(const std::string& path, const std::string& token, const std::string& job_id);
    std::shared_ptr<JobResult> DeleteJob(const std::string& path, const std::string& token, const std::string& job_id);

private:
    static const std::string kAuthorizationHeader;
    static const std::string kContentTypeHeader;
    static const std::string kAcceptHeader;

    std::string runtime_url_;
    std::string transpile_url_;
    web::http::client::http_client runtime_http_client_;
    web::http::client::http_client transpile_http_client_;
};

}  // namespace rexrest

#endif  // IBMQ_REST_HPP_

