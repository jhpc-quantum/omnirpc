#include <chrono>
#include <thread>

#include "ibmq_rest.hpp"

namespace rexrest {

const std::uint32_t IBMQClient::kPollingInterval = 900000;
const std::uint32_t IBMQClient::kMaxPollingCount = 20;

const std::string IBMQHttpClient::kTranspilerServiceURL = "https://cloud-transpiler.quantum.ibm.com";

const std::string IBMQHttpClient::kBackend = "ibm_brisbane";
const std::string IBMQHttpClient::kHub = "ibm-q";
const std::string IBMQHttpClient::kGroup = "open";
const std::string IBMQHttpClient::kProject = "main";

const std::string IBMQHttpClient::kTranspilePath = "/transpile";
const std::string IBMQHttpClient::kGetTranspilationResultsPath = "/transpile/";
const std::string IBMQHttpClient::kListJobDetailsPath = "/jobs/";
const std::string IBMQHttpClient::kListJobResultsPath = "/jobs/";
const std::string IBMQHttpClient::kRunJobPath = "/jobs";

const std::string IBMQHttpClient::kAuthorizationHeader = "Authorization";
const std::string IBMQHttpClient::kContentTypeHeader = "Content-Type";
const std::string IBMQHttpClient::kAcceptHeader = "Accept";

const std::string IBMQHttpClient::kIDKey = "id";
const std::string IBMQHttpClient::kTaskIDKey = "task_id";
const std::string IBMQHttpClient::kStatusKey = "status";
const std::string IBMQHttpClient::kStateKey = "state";
const std::string IBMQHttpClient::kReasonKey = "reason";
const std::string IBMQHttpClient::kResultKey = "result";
const std::string IBMQHttpClient::kQASMKey = "qasm";

const std::string IBMQHttpClient::kStatusCompletedValue = "Completed";
const std::string IBMQHttpClient::kStatusFailedValue = "Failed";
const std::string IBMQHttpClient::kStatusCancelledValue = "Cancelled";
const std::string IBMQHttpClient::kStateSuccessValue = "SUCCESS";
const std::string IBMQHttpClient::kStateFailureValue = "FAILURE";

std::shared_ptr<JobResult> ParseResponse(http_response &res, const std::string &request_type = "",
                                                const std::map<status_code, std::string> &status_messages = {}) {
    if (res.status_code() == status_codes::OK) {
        auto resString = res.extract_string().get();
        return std::make_shared<JobResult>(res.status_code(), resString, "");
    }

    for (auto const& it: status_messages) {
        if (res.status_code() == it.first) {
            // for debug
            if (request_type.length() > 0) {
                std::cerr << request_type << " ";
            }
            std::cerr << "Error: " << res.status_code() << std::endl;

            return std::make_shared<JobResult>(res.status_code(), "", it.second);
        }
    }

    // for debug
    if (request_type.length() > 0) {
        std::cerr << request_type << " ";
    }
    std::cerr << "Error: " << res.status_code() << std::endl;

    return std::make_shared<JobResult>(res.status_code(), "", "Unknown status code");
}

std::shared_ptr<JobResult> IBMQClient::Calculate(const std::string& base_url, const std::string& token,
                                                 const std::string& qasm, const std::uint32_t shots,
                                                 const std::uint32_t transpiler, const std::string& remark,
                                                 std::uint32_t polling_interval, std::uint32_t max_polling_count) {
    // TODO: Add parameter checks

    std::shared_ptr<IBMQHttpClient> client = IBMQHttpClient::CreateHttpClient(base_url);

    // submit transpile job
    std::string transpiled_qasm = "";
    std::shared_ptr<JobResult> transpileResult = client->TranspileCircuit(token, qasm, static_cast<IBMQTranspiler>(transpiler));
    if (transpileResult->GetStatusCode() == status_codes::OK) {
        auto transpileResJson = json::value::parse(transpileResult->GetJsonBody());
        auto taskId = transpileResJson[IBMQHttpClient::kTaskIDKey].as_string();

        // for debug
        std::cerr << transpileResult->GetJsonBody().c_str() << std::endl;

        for (std::uint32_t i = 0; i < max_polling_count; i++) {
            std::shared_ptr<JobResult> transpilationResult = client->GetTranspilationResults(token, taskId);
            if (transpilationResult->GetStatusCode() == status_codes::OK) {
                auto resJson = json::value::parse(transpilationResult->GetJsonBody());
                auto state = resJson[IBMQHttpClient::kStateKey].as_string();

                // for debug
                std::cerr << transpilationResult->GetJsonBody().c_str() << std::endl;

                if (state == IBMQHttpClient::kStateSuccessValue) {
                    transpiled_qasm = resJson[IBMQHttpClient::kResultKey][0][IBMQHttpClient::kQASMKey].as_string();
                    break;
                } else if (state == IBMQHttpClient::kStateFailureValue) {
                    // for debug
                    std::cerr << "Transpile Failed: " << resJson[IBMQHttpClient::kResultKey].as_string().c_str() << std::endl;
                    return std::make_shared<JobResult>(0, "", resJson[IBMQHttpClient::kResultKey].as_string());
                } else {
                    // for debug
                    std::cerr << "Transpile Wait: " << i << std::endl;

                    std::this_thread::sleep_for(std::chrono::milliseconds(polling_interval));
                    continue;
                }
            } else {
                return transpilationResult;
            }
        }
    } else {
        return transpileResult;
    }

    // submit job
    std::shared_ptr<JobResult> runResult = client->RunJob(IBMQHttpClient::kRunJobPath, token, transpiled_qasm, shots, remark);

    if (runResult->GetStatusCode() == status_codes::OK) {
        auto runResJson = json::value::parse(runResult->GetJsonBody());
        auto jobId = runResJson[IBMQHttpClient::kIDKey].as_string();

        // for debug
        std::cerr << runResult->GetJsonBody().c_str() << std::endl;

        for (std::uint32_t i = 0; i < max_polling_count; i++) {
            std::shared_ptr<JobResult> detailsResult = client->ListJobDetails(IBMQHttpClient::kListJobDetailsPath, token, jobId);
            if (detailsResult->GetStatusCode() == status_codes::OK) {
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

std::shared_ptr<JobResult> IBMQHttpClient::TranspileCircuit(const std::string &token, const std::string &qasm,
                                                            const IBMQTranspiler transpiler)
{
    // create path
    uri_builder builder(this->transpile_url_);
    builder.set_path(kTranspilePath);
    builder.append_query("backend=" + kBackend);
    builder.append_query("optimization_level=3");
    builder.append_query("ai=" + IBMQTranspilerUseAI(transpiler));

    // create request
    http_request req(methods::POST);
    req.set_request_uri(builder.to_string());
    req.headers().add(kAuthorizationHeader, "Bearer " + token);
    req.headers().add(kContentTypeHeader, "application/json");
    req.headers().add(kAcceptHeader, "application/json");

    // create JSON
    // https://docs.quantum.ibm.com/api/qiskit-transpiler-service-rest/tags/transpiler-methods#tags__transpiler-methods__operations__transpile_transpile_post
    json::value reqJson;

    json::value circuitsJson;
    circuitsJson[0] = json::value::string(qasm);

    reqJson["qasm_circuits"] = circuitsJson;

    req.set_body(reqJson.serialize());

    // send request
    http_response res = this->transpile_http_client_.request(req).get();
    return ParseResponse(res, "Transpile", {{status_codes::UnprocessableEntity, "Validation Error"}});
}

std::shared_ptr<JobResult> IBMQHttpClient::GetTranspilationResults(const std::string &token, const std::string &task_id)
{
    // create path
    uri_builder builder(this->transpile_url_);
    builder.set_path(kTranspilePath);
    builder.append_path(task_id);

    // create request
    http_request req(methods::GET);
    req.set_request_uri(builder.to_string());
    req.headers().add(kAuthorizationHeader, "Bearer " + token);
    req.headers().add(kAcceptHeader, "application/json");

    // send request
    http_response res = this->transpile_http_client_.request(req).get();
    return ParseResponse(res, "GetTranspilationResults", {{status_codes::UnprocessableEntity, "Validation Error"}});
}

std::shared_ptr<JobResult> IBMQHttpClient::ListJobDetails(const std::string &path,
                                                          const std::string &token,
                                                          const std::string &job_id)
{
    // TODO: Add parameter checks

    // create path
    uri_builder builder(this->runtime_url_);
    builder.set_path(path);
    builder.append_path(job_id);

    // create request
    http_request req(methods::GET);
    req.set_request_uri(builder.to_string());
    req.headers().add(kAuthorizationHeader, "Bearer " + token);
    req.headers().add(kAcceptHeader, "application/json");

    http_response res = this->runtime_http_client_.request(req).get();
    return ParseResponse(res, "ListJobDetails", {
                                                    {status_codes::Unauthorized, "Unauthorized"},
                                                    {status_codes::Forbidden, "Forbidden"},
                                                    {status_codes::NotFound, "Not Found"},
                                                    {status_codes::InternalError, "Internal Error"},
                                                });
}

std::shared_ptr<JobResult> IBMQHttpClient::ListJobResults(const std::string& path,
                                                          const std::string& token,
                                                          const std::string& job_id) {
    // TODO: Add parameter checks

    // create path
    uri_builder builder(this->runtime_url_);
    builder.set_path(path);
    builder.append_path(job_id);
    builder.append_path("results");

    // create request
    http_request req(methods::GET);
    req.set_request_uri(builder.to_string());
    req.headers().add(kAuthorizationHeader, "Bearer " + token);
    req.headers().add(kAcceptHeader, "application/json");

    // send request
    http_response res = this->runtime_http_client_.request(req).get();
    return ParseResponse(res, "ListJobResults", {
                                                    {status_codes::NoContent, "No Content"},
                                                    {status_codes::BadRequest, "Bad Request"},
                                                    {status_codes::Unauthorized, "Unauthorized"},
                                                    {status_codes::Forbidden, "Forbidden"},
                                                    {status_codes::NotFound, "Not Found"},
                                                    {status_codes::InternalError, "Internal Error"},
                                                });
}

std::shared_ptr<JobResult> IBMQHttpClient::RunJob(const std::string& path, const std::string& token,
                                                  const std::string& qasm, const std::uint32_t shots,
                                                  const std::string& remark) {
    // TODO: Add parameter checks

    // create path
    uri_builder builder(this->runtime_url_);
    builder.set_path(path);

    // create request
    http_request req(methods::POST);
    req.set_request_uri(builder.to_string());
    req.headers().add(kAuthorizationHeader, "Bearer " + token);
    req.headers().add(kContentTypeHeader, "application/json");
    req.headers().add(kAcceptHeader, "application/json");

    // create JSON
    //
    // https://docs.quantum.ibm.com/api/runtime/tags/jobs#tags__jobs__operations__CreateJobController_createJob
    json::value reqJson;
    reqJson["program_id"] = json::value::string("sampler");
    reqJson["backend"] = json::value::string(kBackend); // actual machine settings: ibm_brisbane
    reqJson["hub"] = json::value::string(kHub);    // JSON elements not required for IBM Cloud services
    reqJson["group"] = json::value::string(kGroup);   // JSON elements not required for IBM Cloud services
    reqJson["project"] = json::value::string(kProject); // JSON elements not required for IBM Cloud services

    json::value pubsJson;
    pubsJson[0][0] = json::value::string(qasm);

    json::value paramsJson;
    paramsJson["pubs"] = pubsJson;
    paramsJson["shots"] = json::value::number(shots);
    paramsJson["version"] = json::value::number(2);

    reqJson["params"] = paramsJson;

    req.set_body(reqJson.serialize());

    http_response res = this->runtime_http_client_.request(req).get();
    return ParseResponse(res, "RunJob", {
                                            {status_codes::BadRequest, "Bad Request"},
                                            {status_codes::Unauthorized, "Unauthorized"},
                                            {status_codes::NotFound, "Not Found"},
                                            {status_codes::Conflict, "Conflict"},
                                            {status_codes::InternalError, "Internal Error"},
                                        });
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

