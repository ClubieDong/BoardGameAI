#include "Utilities.hpp"
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

static std::shared_mutex MtxJsonValidatorMap;
static std::unordered_map<std::string, nlohmann::json_schema::json_validator> JsonValidatorMap;

const nlohmann::json_schema::json_validator &Util::GetJsonValidator(const std::string &path) {
    {
        const std::shared_lock lock(MtxJsonValidatorMap);
        const auto iter = JsonValidatorMap.find(path);
        if (iter != JsonValidatorMap.end())
            return iter->second;
    }
    nlohmann::json_schema::json_validator validator = {
        nlohmann::json::parse(std::ifstream("schema/" + path)),
        [](const nlohmann::json_uri &id, nlohmann::json &value) {
            value = nlohmann::json::parse(std::ifstream("schema" + id.path()));
        },
    };
    {
        const std::scoped_lock lock(MtxJsonValidatorMap);
        return JsonValidatorMap.try_emplace(path, std::move(validator)).first->second;
    }
}
