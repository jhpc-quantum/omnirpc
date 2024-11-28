#include "ibmq_json.hpp"

namespace rexrest {

const std::string IBMQJsonParser::kResults = "results";
const std::string IBMQJsonParser::kData = "data";
const std::string IBMQJsonParser::kSamples = "samples";

std::shared_ptr<JsonResult> IBMQJsonParser::ScrapeResponse(const std::string& json_str,
                                                           std::uint32_t shots) {
    std::shared_ptr<IBMQJsonParser> parser = std::make_shared<IBMQJsonParser>(json_str, shots);
    return parser->Scrape();
}

std::shared_ptr<JsonResult> IBMQJsonParser::Scrape() {
    std::map<int, float> counts;
    std::shared_ptr<JsonResult> result = std::make_shared<JsonResult>();
    
    auto json = json::value::parse(this->json_str_);
    auto data = json[kResults][0][kData].as_object();

    // iterate over measured classical registers
    for (auto const& it: data) {
        json::value regData = it.second;

        // iterate over all samples (given as an array of hexadecimal values)
        for (auto const& s: regData["samples"].as_array()) {
            int key = std::strtol(s.as_string().c_str(), nullptr, 16);

            if (counts.find(key) != counts.end()) {
                counts[key] += 1;
            } else {
                counts[key] = 1;
            }
        }
    }

    for (auto const& it: counts) {
        result->AddResult(it.first, it.second);
    }

    return result;
}

}  // namespace rexrest

