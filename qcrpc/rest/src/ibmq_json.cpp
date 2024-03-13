#include "ibmq_json.hpp"

namespace rexrest {

const std::string IBMQJsonParser::kQuasiDists = "quasi_dists";
const std::string IBMQJsonParser::kMetadata = "metadata";
const std::string IBMQJsonParser::kShots = "shots";

std::shared_ptr<JsonResult> IBMQJsonParser::ScrapeResponse(const std::string& json_str,
                                                           std::uint32_t shots) {
    std::shared_ptr<IBMQJsonParser> parser = std::make_shared<IBMQJsonParser>(json_str, shots);
    return parser->Scrape();
}

std::shared_ptr<JsonResult> IBMQJsonParser::Scrape() {
    auto json = json::value::parse(this->json_str_);

    auto metadata = json[IBMQJsonParser::kMetadata].as_array();
    int shots = metadata[0][IBMQJsonParser::kShots].as_integer();

    std::shared_ptr<JsonResult> result = std::make_shared<JsonResult>();

    auto quasiDists = json[IBMQJsonParser::kQuasiDists].as_array();
    auto quasiDistObj = quasiDists[0].as_object();
    for (auto quasiDist = quasiDistObj.cbegin(); quasiDist != quasiDistObj.cend(); ++quasiDist) {
        int key = std::stoi(quasiDist->first, 0, 2);
        float value = std::stof(std::to_string(quasiDist->second.as_double()));

        // Convert probability to number of observations
        result->AddResult(key, value * shots);
    }

    return result;
}

}  // namespace rexrest

