//database.cpp
#include "Database.h"
#include <fstream>
#include <iostream>

void Database::saveToFile(const std::string& filename, const nlohmann::json& data) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Błąd: Nie można zapisać do pliku " << filename << std::endl;
        return;
    }
    file << data.dump(4); // Ładny format JSON
    file.close();
}

nlohmann::json Database::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Błąd: Nie można otworzyć pliku " << filename << std::endl;
        return {};
    }

    nlohmann::json data;
    try {
        file >> data;
    }
    catch (nlohmann::json::parse_error& e) {
        std::cerr << "Błąd parsowania JSON: " << e.what() << std::endl;
    }

    return data;
}
