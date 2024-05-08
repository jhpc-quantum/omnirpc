#ifndef REXREST_COMMON_HPP_
#define REXREST_COMMON_HPP_

#include <vector>

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

class JsonResult {
public:
    JsonResult() : patterns_{}, probs_{}, size_(0) {}
    ~JsonResult() {}

    void AddResult(int pattern, float prob) {
        patterns_.push_back(pattern);
        probs_.push_back(prob);
	size_++;
    }
    std::vector<int> GetPatterns() { return patterns_; };
    std::vector<float> GetProbs() { return probs_; };
    std::uint32_t GetSize() { return size_; };

private:
    std::vector<int> patterns_;
    std::vector<float> probs_;
    std::uint32_t size_;
};

class JsonParser {
public:
    JsonParser(const std::string& json_str, std::uint32_t shots) : json_str_(json_str), shots_(shots) {}
    ~JsonParser() {}

    virtual std::shared_ptr<JsonResult> Scrape() = 0;

protected:
    std::string json_str_;
    std::uint32_t shots_;
};

}  // namespace rexrest

#endif // REXREST_COMMON_HPP_

