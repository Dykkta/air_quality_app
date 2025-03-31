//jsonhandler.cpp
#include "JsonHandler.h"
#include <iostream>

nlohmann::json JsonHandler::parseJson(const std::string& jsonString) {
    try {
        return nlohmann::json::parse(jsonString);
    }
    catch (nlohmann::json::parse_error& e) {
        std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
        return {};
    }
}
