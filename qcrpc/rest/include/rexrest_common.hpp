#ifndef REXREST_COMMON_HPP_
#define REXREST_COMMON_HPP_

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>

using namespace utility;              // Common utilities like string conversions
using namespace web;                  // Common features like URIs.
using namespace web::http;            // Common HTTP functionality
using namespace web::http::client;    // HTTP client features
using namespace concurrency::streams; // Asynchronous streams

namespace rexrest {

class JobResult {
public:
    JobResult(const unsigned short status_code, const std::string json_body)
            : status_code_(status_code), json_body_(json_body), err_msg_("") {}
    JobResult(const unsigned short status_code, const std::string json_body,
              const std::string err_msg)
            : status_code_(status_code), json_body_(json_body), err_msg_(err_msg) {}
    ~JobResult() {}

    unsigned short GetStatusCode() { return status_code_; }
    std::string GetJsonBody() { return json_body_; }
    std::string GetErrMsg() { return err_msg_; }

private:
    unsigned short status_code_;
    std::string json_body_;
    std::string err_msg_;
};

}  // namespace rexrest

#endif // REXREST_COMMON_HPP_

