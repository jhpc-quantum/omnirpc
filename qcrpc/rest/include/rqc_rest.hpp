#ifndef REXREST_RQC_HPP_
#define REXREST_RQC_HPP_

#include "rqc_common.hpp"

namespace rexrest {

class RQCClient {
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

enum class Transpiler : std::uint32_t {
    NONE = 0,
    PASS,
    NORMAL
};

inline const std::string TranspilerToString(Transpiler transpiler) {
  switch (transpiler) {
    case Transpiler::NONE: { return "none"; }
    case Transpiler::PASS: { return "pass"; }
    case Transpiler::NORMAL: { return "normal"; }
    default: { return "none"; }
  }
}

class RQCHttpClient {
public:
    static const std::string kGetJobPath;
    static const std::string kSubmitJobPath;
    static const std::string kIDKey;
    static const std::string kJobIDKey;
    static const std::string kReasonKey;
    static const std::string kStatusKey;
    static const std::string kStatusSuccessValue;
    static const std::string kStatusFailureValue;

    static std::shared_ptr<RQCHttpClient> CreateRQCHttpClient(const std::string& base_url);

    RQCHttpClient(const std::string& base_url) : base_url_(base_url), http_client_(base_url) {}
    ~RQCHttpClient() {}

    std::shared_ptr<JobResult> GetJob(const std::string& path, const std::string& token, const std::string& job_id);
    std::shared_ptr<JobResult> SubmitJob(const std::string& path, const std::string& token, const std::string& qasm,
                  const std::uint32_t shots, const std::string& transpiler, const std::string& remark);
    std::shared_ptr<JobResult> CancelJob(const std::string& path, const std::string& token, const std::string& job_id);
    std::shared_ptr<JobResult> DeleteJob(const std::string& path, const std::string& token, const std::string& job_id);

private:
    static const std::string kQApiTokenHeader;
    static const std::string kContentTypeHeader;

    std::string base_url_;
    web::http::client::http_client http_client_;
};

}  // namespace rexrest

#endif  // REXREST_RQC_HPP_

