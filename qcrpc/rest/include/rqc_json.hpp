#ifndef RQC_JSON_HPP_
#define RQC_JSON_HPP_

#include "rexrest_common.hpp"

namespace rexrest {

class RQCJsonParser : JsonParser {
public:
    static const std::string kResult;
    static const std::string kCounts;

    RQCJsonParser(const std::string& json_str, std::uint32_t shots) : JsonParser(json_str, shots) {}
    ~RQCJsonParser() {}

    virtual std::shared_ptr<JsonResult> Scrape() override;

    static std::shared_ptr<JsonResult> ScrapeResponse(const std::string& json_str, std::uint32_t shots);
};

}  // namespace rexrest

#endif  // RQC_JSON_HPP_

