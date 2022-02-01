#include "Utilities.hpp"
#include <fstream>
#include <mutex>

const nlohmann::json_schema::json_validator &Util::GetJsonValidator(std::string_view path) {
    {
        const std::shared_lock lock(_MtxValidatorMap);
        const auto iter = _ValidatorMap.find(path);
        if (iter != _ValidatorMap.end())
            return iter->second;
    }
    nlohmann::json_schema::json_validator validator = {
        nlohmann::json::parse(std::ifstream("schema/" + std::string(path))),
        [](const nlohmann::json_uri &id, nlohmann::json &value) {
            value = nlohmann::json::parse(std::ifstream("schema" + id.path()));
        },
    };
    {
        const std::scoped_lock lock(_MtxValidatorMap);
        return _ValidatorMap.try_emplace(path, std::move(validator)).first->second;
    }
}
