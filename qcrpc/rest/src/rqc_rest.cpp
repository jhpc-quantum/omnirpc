#include <chrono>
#include <thread>

#include "rqc_rest.hpp"

namespace rexrest {

const std::uint32_t RQCClient::kPollingInterval = 1000;
const std::uint32_t RQCClient::kMaxPollingCount = 60;

const std::string RQCHttpClient::kGetJobPath = "/dev/api/jobs/";
const std::string RQCHttpClient::kSubmitJobPath = "/dev/api/jobs";
const std::string RQCHttpClient::kQApiTokenHeader = "q-api-token";
const std::string RQCHttpClient::kContentTypeHeader = "Content-Type";
const std::string RQCHttpClient::kIDKey = "id";
const std::string RQCHttpClient::kJobIDKey = "job_id";
const std::string RQCHttpClient::kReasonKey = "reason";
const std::string RQCHttpClient::kStatusKey = "status";
const std::string RQCHttpClient::kStatusSuccessValue = "success";
const std::string RQCHttpClient::kStatusFailureValue = "failure";

std::shared_ptr<JobResult> RQCClient::Calculate(const std::string& base_url, const std::string& token,
                                                const std::string& qasm, const std::uint32_t shots,
                                                const std::uint32_t transpiler, const std::string& remark,
                                                std::uint32_t polling_interval, std::uint32_t max_polling_count) {
    // TODO: Add parameter checks

    std::shared_ptr<RQCHttpClient> client = RQCHttpClient::CreateRQCHttpClient(base_url);

    auto transpilerStr = TranspilerToString(static_cast<Transpiler>(transpiler));

    // submit job
    std::shared_ptr<JobResult> submitResult = client->SubmitJob(RQCHttpClient::kSubmitJobPath, token, qasm, shots, transpilerStr, remark);
    if (submitResult->GetStatusCode() == web::http::status_codes::Created) {
        auto submitResJson = json::value::parse(submitResult->GetJsonBody());
        auto jobId = submitResJson[RQCHttpClient::kJobIDKey].as_string();

        // for debug
        std::wcout << submitResult->GetJsonBody().c_str() << std::endl;

        for (std::uint32_t i = 0; i < max_polling_count; i++) {
            std::shared_ptr<JobResult> getResult = client->GetJob(RQCHttpClient::kSubmitJobPath, token, jobId);
            if (getResult->GetStatusCode() == web::http::status_codes::OK) {
                auto getResJson = json::value::parse(getResult->GetJsonBody());
                auto status = getResJson[RQCHttpClient::kStatusKey].as_string();

                // for debug
                std::wcout << getResult->GetJsonBody().c_str() << std::endl;

                if (status == RQCHttpClient::kStatusSuccessValue || status == RQCHttpClient::kStatusFailureValue) {
                    return getResult;
                } else {
                    // for debug
                    std::wcout << "Calculate Wait: " << i << std::endl;

                    std::this_thread::sleep_for(std::chrono::milliseconds(polling_interval));
                    continue;
                }
            } else {
                return getResult;
            }
        }

        // TODO: The value of the status code at the time of error needs to be considered.
        return std::make_shared<JobResult>(0, "", "Calculate(GetJob) Timeout");
    } else {
        return submitResult;
    }
}

std::shared_ptr<RQCHttpClient> RQCHttpClient::CreateRQCHttpClient(const std::string& base_url) {
    return std::make_shared<RQCHttpClient>(base_url);
}

std::shared_ptr<JobResult> RQCHttpClient::GetJob(const std::string& path, const std::string& token,
                                                 const std::string& job_id) {
    // TODO: Add parameter checks

    // create path
    uri_builder builder(this->base_url_);
    builder.set_path(path);
    builder.append_path(job_id);

    // create request
    http_request req(methods::GET);
    req.set_request_uri(builder.to_string());
    req.headers().add(kQApiTokenHeader, token);

    http_response res = this->http_client_.request(req).get();
    switch (res.status_code()) {
        case web::http::status_codes::OK: {
            auto resString = res.extract_string().get();
            return std::make_shared<JobResult>(res.status_code(), resString, "");
        }
        case web::http::status_codes::BadRequest: {
            auto resString = res.extract_string().get();
            auto resJson = json::value::parse(resString);
            return std::make_shared<JobResult>(res.status_code(), "", resJson[kReasonKey].as_string());
        }
        case web::http::status_codes::Unauthorized: {
            std::wcout << "GetJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unauthorized");
        }
        case web::http::status_codes::NotFound: {
            std::wcout << "GetJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Not Found");
        }
        case web::http::status_codes::InternalError: {
            std::wcout << "GetJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Internal Error");
        }
        default: {
            std::wcout << "GetJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unknown status code");
        }
    }
}

std::shared_ptr<JobResult> RQCHttpClient::SubmitJob(const std::string& path, const std::string& token,
                                                    const std::string& qasm, const std::uint32_t shots,
                                                    const std::string& transpiler, const std::string& remark) {
    // TODO: Add parameter checks

    // create path
    uri_builder builder(this->base_url_);
    builder.set_path(path);

    // create request
    http_request req(methods::POST);
    req.set_request_uri(builder.to_string());
    req.headers().add(kQApiTokenHeader, token);
    req.headers().add(kContentTypeHeader, "application/json");

    // create JSON
    json::value reqJson;
    reqJson["qasm"] = json::value::string(qasm);
    reqJson["shots"] = json::value::number(shots);
    reqJson["transpiler"] = json::value::string(transpiler);
    reqJson["remark"] = json::value::string(remark);

    req.set_body(reqJson.serialize());

    http_response res = this->http_client_.request(req).get();
    switch (res.status_code()) {
        case web::http::status_codes::Created: {
            auto resString = res.extract_string().get();
            return std::make_shared<JobResult>(res.status_code(), resString, "");
        }
        case web::http::status_codes::BadRequest: {
            std::wcout << "SubmitJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Bad Request");
        }
        case web::http::status_codes::Unauthorized: {
            std::wcout << "SubmitJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unauthorized");
        }
        case web::http::status_codes::NotFound: {
            std::wcout << "SubmitJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Not Found");
        }
        case web::http::status_codes::InternalError: {
            std::wcout << "SubmitJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Internal Error");
        }
        default: {
            std::wcout << "SubmitJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unknown status code");
        }
    }
}

std::shared_ptr<JobResult> RQCHttpClient::CancelJob(const std::string& path, const std::string& token,
                                                    const std::string& job_id) {
    // Unsupported operation
    return std::make_shared<JobResult>(0, "", "Unsupported operation");
}

std::shared_ptr<JobResult> RQCHttpClient::DeleteJob(const std::string& path, const std::string& token,
                                                    const std::string& job_id) {
    // Unsupported operation
    return std::make_shared<JobResult>(0, "", "Unsupported operation");
}

}  // namespace rexrest

