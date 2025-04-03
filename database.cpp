/**
 * @file Database.cpp
 * @brief Implementacja klasy Database służącej do obsługi operacji zapisu i odczytu danych JSON
 * @details Klasa zawiera funkcjonalność do zapisywania i odczytywania danych o jakości powietrza w formacie JSON
 */

#include "Database.h"
#include <fstream>
#include <iostream>

/**
 * @brief Zapisuje dane JSON do pliku
 * @param filename Nazwa pliku docelowego
 * @param data Dane w formacie JSON do zapisania
 * @details Funkcja zapisuje dane JSON z ładnym formatowaniem (wcięcie 4 spacje)
 */
void Database::saveToFile(const std::string& filename, const nlohmann::json& data) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Błąd: Nie można zapisać do pliku " << filename << std::endl;
        return;
    }
    file << data.dump(4); // Ładny format JSON
    file.close();
}

/**
 * @brief Wczytuje dane JSON z pliku
 * @param filename Nazwa pliku źródłowego
 * @return Obiekt JSON zawierający wczytane dane lub pusty obiekt w przypadku błędu
 * @details Funkcja obsługuje błędy otwarcia pliku oraz parsowania formatu JSON
 */
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

/**
 * @brief Sprawdza czy plik istnieje
 * @param filename Nazwa pliku do sprawdzenia
 * @return true jeśli plik istnieje, false w przeciwnym wypadku
 */
bool Database::fileExists(const std::string& filename) {
    return std::filesystem::exists(filename);
}
