//jsonhandler.h
#pragma once
#include <string>
#include <nlohmann/json.hpp>

/**
 * @brief Klasa do parsowania i walidacji danych JSON
 *
 * Klasa JsonHandler zapewnia funkcjonalność do przetwarzania ciągów znaków
 * w formacie JSON do obiektów klasy nlohmann::json. Wykorzystuje parser biblioteki
 * nlohmann/json do efektywnego przetwarzania danych.
 */
class JsonHandler {
public:
    /**
     * @brief Parsuje ciąg znaków JSON do obiektu
     * @param jsonString Ciąg znaków w formacie JSON do sparsowania
     * @return Sparsowany obiekt JSON
     * @throw nlohmann::json::parse_error W przypadku nieprawidłowego formatu JSON
     * @throw std::invalid_argument W przypadku pustego ciągu wejściowego
     *
     * Metoda konwertuje ciąg znaków w formacie JSON na obiekt json, który może być
     * następnie łatwo przetwarzany. Przykład użycia:
     * @code
     * try {
     *     auto data = JsonHandler::parseJson("{\"key\":\"value\"}");
     * } catch(const nlohmann::json::parse_error& e) {
     *     // obsługa błędów parsowania
     * }
     * @endcode
     *
     * @note Metoda jest wrapperm wokół nlohmann::json::parse, zapewniającym dodatkową
     * walidację wejścia i ujednolicone zarządzanie błędami.
     */
    static nlohmann::json parseJson(const std::string& jsonString);
};
