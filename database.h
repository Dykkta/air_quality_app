//database.h
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class Database {
public:
    static void saveToFile(const std::string& filename, const nlohmann::json& data);
    static nlohmann::json loadFromFile(const std::string& filename);
};
