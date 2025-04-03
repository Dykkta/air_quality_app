/**
 * @file JsonHandler.cpp
 * @brief Implementacja klasy JsonHandler do przetwarzania danych JSON
 * @details Klasa odpowiada za parsowanie danych JSON otrzymanych z API jakości powietrza
 */

#include "JsonHandler.h"
#include <iostream>

/**
 * @brief Parsuje string zawierający dane JSON do obiektu nlohmann::json
 * @param jsonString String zawierający dane w formacie JSON
 * @return Sparsowany obiekt nlohmann::json lub pusty obiekt w przypadku błędu parsowania
 * @details Funkcja obsługuje wyjątki związane z parsowaniem JSON i raportuje błędy do strumienia błędów
 */
nlohmann::json JsonHandler::parseJson(const std::string& jsonString) {
    try {
        return nlohmann::json::parse(jsonString);
    }
    catch (nlohmann::json::parse_error& e) {
        std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
        return {};
    }
}
