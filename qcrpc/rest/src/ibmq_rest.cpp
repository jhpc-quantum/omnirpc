#include <chrono>
#include <thread>

#include "ibmq_rest.hpp"

namespace rexrest {

const std::uint32_t IBMQClient::kPollingInterval = 900000;
const std::uint32_t IBMQClient::kMaxPollingCount = 20;

const std::string IBMQHttpClient::kListJobDetailsPath = "/jobs/";
const std::string IBMQHttpClient::kListJobResultsPath = "/jobs/";
const std::string IBMQHttpClient::kRunJobPath = "/jobs";
const std::string IBMQHttpClient::kAuthorizationHeader = "Authorization";
const std::string IBMQHttpClient::kContentTypeHeader = "Content-Type";
const std::string IBMQHttpClient::kIDKey = "id";
const std::string IBMQHttpClient::kStatusKey = "status";
const std::string IBMQHttpClient::kStatusCompletedValue = "Completed";
const std::string IBMQHttpClient::kStatusFailedValue = "Failed";
const std::string IBMQHttpClient::kStatusCancelledValue = "Cancelled";
const std::string IBMQHttpClient::kStateKey = "state";
const std::string IBMQHttpClient::kReasonKey = "reason";

std::shared_ptr<JobResult> IBMQClient::Calculate(const std::string& base_url, const std::string& token,
                                                 const std::string& qasm, const std::uint32_t shots,
                                                 const std::uint32_t transpiler, const std::string& remark,
                                                 std::uint32_t polling_interval, std::uint32_t max_polling_count) {
    // TODO: Add parameter checks

    std::shared_ptr<IBMQHttpClient> client = IBMQHttpClient::CreateHttpClient(base_url);

    auto transpilerStr = IBMQTranspilerToString(static_cast<IBMQTranspiler>(transpiler));

    // submit job
    std::shared_ptr<JobResult> runResult = client->RunJob(IBMQHttpClient::kRunJobPath, token, qasm, shots, transpilerStr, remark);

    if (runResult->GetStatusCode() == web::http::status_codes::OK) {
        auto runResJson = json::value::parse(runResult->GetJsonBody());
        auto jobId = runResJson[IBMQHttpClient::kIDKey].as_string();

        // for debug
        std::cerr << runResult->GetJsonBody().c_str() << std::endl;

        for (std::uint32_t i = 0; i < max_polling_count; i++) {
            std::shared_ptr<JobResult> detailsResult = client->ListJobDetails(IBMQHttpClient::kListJobDetailsPath, token, jobId);
            if (detailsResult->GetStatusCode() == web::http::status_codes::OK) {
                auto getResJson = json::value::parse(detailsResult->GetJsonBody());
                auto status = getResJson[IBMQHttpClient::kStatusKey].as_string();

                // for debug
                std::cerr << detailsResult->GetJsonBody().c_str() << std::endl;

                if (status == IBMQHttpClient::kStatusCompletedValue) {
                    return client->ListJobResults(IBMQHttpClient::kListJobResultsPath, token, jobId);
                } else if (status == IBMQHttpClient::kStatusFailedValue) {
                    // for debug
                    std::cerr << "Job Failed: " << getResJson[IBMQHttpClient::kStateKey][IBMQHttpClient::kReasonKey].as_string().c_str() << std::endl;
                    return std::make_shared<JobResult>(0, "", getResJson[IBMQHttpClient::kStateKey][IBMQHttpClient::kReasonKey].as_string());
                } else if (status == IBMQHttpClient::kStatusCancelledValue) {
                    // for debug
                    std::cerr << "Job Cancelled" << std::endl;
                    return std::make_shared<JobResult>(0, "", "Cancelled");
                } else {
                    // for debug
                    std::cerr << "Calculate Wait: " << i << std::endl;

                    std::this_thread::sleep_for(std::chrono::milliseconds(polling_interval));
                    continue;
                }
            } else {
                return detailsResult;
            }
        }

        // TODO: The value of the status code at the time of error needs to be considered.
        return std::make_shared<JobResult>(0, "", "Calculate(GetJob) Timeout");
    } else {
        return runResult;
    }
}

std::shared_ptr<JobResult> IBMQHttpClient::ListJobDetails(const std::string& path,
                                                          const std::string& token,
                                                          const std::string& job_id) {
    // TODO: Add parameter checks

    // create path
    uri_builder builder(this->base_url_);
    builder.set_path(path);
    builder.append_path(job_id);

    // create request
    http_request req(methods::GET);
    req.set_request_uri(builder.to_string());
    req.headers().add(kAuthorizationHeader, "Bearer " + token);

    http_response res = this->http_client_.request(req).get();
    switch (res.status_code()) {
        case web::http::status_codes::OK: {
            auto resString = res.extract_string().get();
            return std::make_shared<JobResult>(res.status_code(), resString, "");
        }

        case web::http::status_codes::Unauthorized: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unauthorized");
        }
        case web::http::status_codes::Forbidden: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Forbidden");
        }
        case web::http::status_codes::NotFound: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Not Found");
        }
        case web::http::status_codes::InternalError: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Internal Error");
        }
        default: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unknown status code");
        }
    }
}

std::shared_ptr<JobResult> IBMQHttpClient::ListJobResults(const std::string& path,
                                                          const std::string& token,
                                                          const std::string& job_id) {
    // TODO: Add parameter checks

    // create path
    uri_builder builder(this->base_url_);
    builder.set_path(path);
    builder.append_path(job_id);
    builder.append_path("results");

    // create request
    http_request req(methods::GET);
    req.set_request_uri(builder.to_string());
    req.headers().add(kAuthorizationHeader, "Bearer " + token);

    http_response res = this->http_client_.request(req).get();
    switch (res.status_code()) {
        case web::http::status_codes::OK: {
            auto resString = res.extract_string().get();
            return std::make_shared<JobResult>(res.status_code(), resString, "");
        }
        case web::http::status_codes::NoContent: {
            std::cerr << "ListJobResults Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "No Content");
        }
        case web::http::status_codes::BadRequest: {
            std::cerr << "ListJobResults Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Bad Request");
        }
        case web::http::status_codes::Unauthorized: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unauthorized");
        }
        case web::http::status_codes::Forbidden: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Forbidden");
        }
        case web::http::status_codes::NotFound: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Not Found");
        }
        case web::http::status_codes::InternalError: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Internal Error");
        }
        default: {
            std::cerr << "ListJobDetails Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unknown status code");
        }
    }
}

std::shared_ptr<JobResult> IBMQHttpClient::RunJob(const std::string& path, const std::string& token,
                                                  const std::string& qasm, const std::uint32_t shots,
                                                  const std::string& transpiler, const std::string& remark) {
    // TODO: Add parameter checks

    // create path
    uri_builder builder(this->base_url_);
    builder.set_path(path);

    // create request
    http_request req(methods::POST);
    req.set_request_uri(builder.to_string());
    req.headers().add(kAuthorizationHeader, "Bearer " + token);
    req.headers().add(kContentTypeHeader, "application/json");

    // create JSON
    //
    // https://docs.quantum.ibm.com/api/runtime/tags/jobs#tags__jobs__operations__CreateJobController_createJob
    json::value reqJson;
    reqJson["program_id"] = json::value::string("sampler");
    reqJson["backend"] = json::value::string("ibmq_qasm_simulator"); // actual machine settings: ibm_brisbane
    reqJson["hub"] = json::value::string("ibm-q");    // JSON elements not required for IBM Cloud services
    reqJson["group"] = json::value::string("open");   // JSON elements not required for IBM Cloud services
    reqJson["project"] = json::value::string("main"); // JSON elements not required for IBM Cloud services

    json::value paramsJson;
    paramsJson[0] = json::value::string(qasm);

    json::value circuitsJson;
    circuitsJson["circuits"] = paramsJson;

    reqJson["params"] = circuitsJson;

    req.set_body(reqJson.serialize());

    http_response res = this->http_client_.request(req).get();
    switch (res.status_code()) {
        case web::http::status_codes::OK: {
            auto resString = res.extract_string().get();
            return std::make_shared<JobResult>(res.status_code(), resString, "");
        }
        case web::http::status_codes::BadRequest: {
            std::cerr << "RunJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Bad Request");
        }
        case web::http::status_codes::Unauthorized: {
            std::cerr << "RunJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unauthorized");
        }
        case web::http::status_codes::NotFound: {
            std::cerr << "RunJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Not Found");
        }
        case web::http::status_codes::Conflict: {
            std::cerr << "RunJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Conflict");
        }
        case web::http::status_codes::InternalError: {
            std::cerr << "RunJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Internal Error");
        }
        default: {
            std::cerr << "RunJob Error: " << res.status_code() << std::endl;
            return std::make_shared<JobResult>(res.status_code(), "", "Unknown status code");
        }
    }
}

std::shared_ptr<JobResult> IBMQHttpClient::CancelJob(const std::string& path, const std::string& token,
                                                     const std::string& job_id) {
    // Unsupported operation
    return std::make_shared<JobResult>(0, "", "Unsupported operation");
}

std::shared_ptr<JobResult> IBMQHttpClient::DeleteJob(const std::string& path, const std::string& token,
                                                     const std::string& job_id) {
    // Unsupported operation
    return std::make_shared<JobResult>(0, "", "Unsupported operation");
}

std::shared_ptr<IBMQHttpClient> IBMQHttpClient::CreateHttpClient(const std::string& base_url) {
    return std::make_shared<IBMQHttpClient>(base_url);
}

}  // namespace rexrest

