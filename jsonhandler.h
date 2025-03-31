//jsonhandler.h
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class JsonHandler {
public:
    static nlohmann::json parseJson(const std::string& jsonString);
};
