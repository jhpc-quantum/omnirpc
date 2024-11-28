#ifndef IBMQ_JSON_HPP_
#define IBMQ_JSON_HPP_

#include "rexrest_common.hpp"

namespace rexrest {

class IBMQJsonParser : JsonParser {
public:
    static const std::string kResults;
    static const std::string kData;
    static const std::string kSamples;

    IBMQJsonParser(const std::string& json_str, std::uint32_t shots) : JsonParser(json_str, shots) {}
    ~IBMQJsonParser() {}

    virtual std::shared_ptr<JsonResult> Scrape() override;

    static std::shared_ptr<JsonResult> ScrapeResponse(const std::string& json_str, std::uint32_t shots);
};

}  // namespace rexrest

#endif  // IBMQ_JSON_HPP_

